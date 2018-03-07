/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package tank;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.Socket;
import java.util.Date;
import javax.swing.JOptionPane;

/**
 *
 * @author alexey
 */
public class ReadServer
        extends Thread {

    private final javax.swing.JTextField stateField;    //Поле вывода ошибок
    private final Socket client;    //Клиентский сокет
    private final mainJFrame window;    //Главное окно

    public ReadServer(javax.swing.JTextField Field, Socket clnt, mainJFrame parent) {
        stateField = Field;
        client = clnt;
        window = parent;
    }

    @Override
    public void run() //Этот метод будет выполняться в побочном потоке
    {
        try {
            BufferedReader br = new BufferedReader(new InputStreamReader(client.getInputStream()));
            String msg;
            while ((msg = br.readLine()) != null) {
                if (msg.startsWith("PARAM")) {
                        String[] lol;
                        lol = msg.substring(6).split(" ");
                        window.getParam(Integer.parseInt(lol[0]), Integer.parseInt(lol[1]),
                                Integer.parseInt(lol[2]), Integer.parseInt(lol[3]), 
                                Integer.parseInt(lol[4]), Integer.parseInt(lol[5]),
                                Integer.parseInt(lol[6]));
                }
                else{
                Date time = new Date(); //Момент получения сообщения
                stateField.setText(msg + "\t" + time.toString()); //Вывод полученного сообщения
                time = null;
                }
                
                //JOptionPane.showMessageDialog(stateField,msg);
            }
            window.closeClient();
        } catch (Exception e) {
            //mainJFrame.
            JOptionPane.showMessageDialog(window, e);
            window.closeClient();
        }
    }
}
