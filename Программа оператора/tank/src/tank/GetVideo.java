/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package tank;

import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileOutputStream;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import javax.imageio.ImageIO;
import javax.swing.JOptionPane;

/**
 *
 * @author alexey
 */
public class GetVideo
        extends Thread {

    private final java.awt.Canvas canvas1;
    private DatagramSocket sk = null;

    public GetVideo(java.awt.Canvas canv) {
        canvas1 = canv;
    }

    @Override
    public void run() //Этот метод будет выполняться в побочном потоке
    {
        File imgFile = null; //Временный файл для хранения кадра
        try {
            final int UDPport = 9999;
            byte[] buf = new byte[1000];
            DatagramPacket dgp = new DatagramPacket(buf, buf.length);
            sk = new DatagramSocket(UDPport);
            sk.setSoTimeout(4000);
            imgFile = File.createTempFile("img2", ".jpg"); //Временный файл для хранения кадра
            //working = true;
            while (!this.isInterrupted()) {
                try {
                    FileOutputStream file = new FileOutputStream(imgFile);
                    while (!this.isInterrupted()) {
                    //Получаем дейтаграмму
                        //Если там строка, что передача окончена, перестаём писать.
                        //Пишем все дейтаграммы подряд в файл.
                        sk.receive(dgp);
                        buf = dgp.getData();
                        String rcvd = new String(buf, 0, dgp.getLength());
                        if (rcvd.intern() == "stop\n") {
                            break;
                        }
                        file.write(buf);
                    }
                    file.close();
                    file=null;
                    BufferedImage bimg = ImageIO.read(imgFile);
                    canvas1.getGraphics().drawImage(bimg, 0, 0, null);
                    bimg=null;
                    System.gc();
                } catch (Exception e) {
                    System.out.println(e);
                }
            }
        } catch (Exception e) {
            JOptionPane.showMessageDialog(canvas1, e);
            //working = false;
        } finally {
            canvas1.getGraphics().clearRect(0, 0, canvas1.getWidth(), canvas1.getHeight());
            if (imgFile != null) {
                if (imgFile.exists()) {
                    imgFile.delete();
                }
            }
            if (sk != null) {
                sk.disconnect();
                sk.close();
            }
        }
    }
}