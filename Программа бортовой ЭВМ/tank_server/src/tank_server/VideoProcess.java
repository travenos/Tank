/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package tank_server;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.PrintStream;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import static org.bytedeco.javacpp.opencv_core.*;
import static org.bytedeco.javacpp.opencv_highgui.*;
import org.bytedeco.javacpp.opencv_imgproc;
import static org.bytedeco.javacpp.opencv_imgproc.CV_BGR2HSV;
import static org.bytedeco.javacpp.opencv_imgproc.CV_MEDIAN;
import static org.bytedeco.javacpp.opencv_imgproc.cvCvtColor;
import static org.bytedeco.javacpp.opencv_imgproc.cvMoments;
import static org.bytedeco.javacpp.opencv_imgproc.cvSmooth;

/**
 *
 * @author alexey
 */
public class VideoProcess
        extends Thread {

    final private PrintStream printStream;  //Поток отправки текстовых данных
    IplImage IplImg = null; //Кадр в формате IplImage
    private final InetAddress firstClientAddr; //Адрес, куда передаётся видео

    private boolean captureWorking = false; //Флаг показывает, ведётся ли захват видео с камеры    
    private boolean broadcastWorking = false; //Флаг показывает, ведётся ли трансляция   
    private boolean trackingWorking = false; //Флаг показывает, ведётся ли слежение за точкой
    private boolean adjustWorking = false; //Флаг показывает, выполняется ли определение параметров камеры
    private boolean driveToPoint = true; //Флаг показывает, ехать ли к найденной точке

    private int adjustXSumm = 0; //Массив измерений горизонтальной координаты
    private int adjustYSumm = 0; //Массив измерений вертикальной координаты
    private int adjustCount = 0; //Подсчёт сделанных измерений
    private final double adjustDist = 1;
    private final double adjustRight = 0.2;

    private long timeDelay = Tank_server.td; //Текущее время обработки

    //private final CvScalar min = CV_RGB(140, 0, 0);   //Минимальная интенсивность точки в RGB
    //private final CvScalar max = CV_RGB(255, 90, 110);   //Максимальная интенсивность точки в RGB
    private static CvScalar min = cvScalar(170, 150, 100, 0);   //Минимальная интенсивность точки в HSB
    private static CvScalar max = cvScalar(179, 255, 255, 0);   //Максимальная интенсивность точки в HSB

    private int width = 640; //Ширина изображения
    private int height = 480; //Высота изображения
    private AutoSystem SAU;

    public VideoProcess(PrintStream ps, InetAddress addr) //Конструктор
    {
        printStream = ps;
        firstClientAddr = addr;
    }

    @Override
    public void run() //Этот метод будет выполняться в побочном потоке
    {
        CvCapture capture = cvCreateCameraCapture(CV_CAP_ANY);
        width = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
        height = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
        try {
            final File imgFile = File.createTempFile("img", ".jpg"); //Временный файл для хранения кадра
            final String image = imgFile.getAbsolutePath();
            final int UDPport = 9999; //Порт UDP-сервера
            DatagramSocket dtgSkt = new DatagramSocket();  //UDP-сокет для отправки виедо          
            byte[] buf = new byte[1000]; //Буфер видеоданных
            int[] p = {CV_IMWRITE_JPEG_QUALITY, 40, 0}; //Качество jpeg

            captureWorking = true;
            while (captureWorking) {
                long starttime = System.currentTimeMillis();
                IplImg = cvQueryFrame(capture); //Получение текущего кадра
                if (trackingWorking || adjustWorking) { //Обработка видео
                    imgTrack();
                }
                if (broadcastWorking) {  //Трансляция видео 
                    cvSaveImage(image, IplImg, p);
                    FileInputStream file = new FileInputStream(image);
                    final int size = file.available();
                    DatagramPacket out;
                    for (int i = 1; i <= size / buf.length + 1; i++) {
                        file.read(buf, 0, (size < buf.length * i) ? (size - buf.length * (i - 1)) : buf.length);
                        out = new DatagramPacket(buf, (size < buf.length * i) ? (size - buf.length * (i - 1)) : buf.length, firstClientAddr, UDPport);
                        dtgSkt.send(out);
                    }

                    out = new DatagramPacket("stop\n".getBytes(), "stop\n".getBytes().length, firstClientAddr, UDPport);
                    dtgSkt.send(out);
                }
                if (SAU != null && adjustWorking) { //Суммирование координат для вычисления средних при автонастройке
                    adjustXSumm += SAU.getX();
                    adjustYSumm += SAU.getY();
                    adjustCount++;
                }
                IplImg.release();
                IplImg = null;
                System.gc(); //Запрос сборщика мусора
                long stoptime = System.currentTimeMillis();
                timeDelay = (Tank_server.td > stoptime - starttime) ? Tank_server.td : stoptime - starttime;
                long sleeptime = Tank_server.td - timeDelay;
                //Потом убрать
                //  System.out.println(stoptime - starttime);
                //
                if (sleeptime > 0) {
                    sleep(sleeptime);
                }
            }
            cvReleaseCapture(capture);
            if (imgFile.exists()) {
                imgFile.delete();
            }
        } catch (Exception e) {
            try {
                if (captureWorking) {
                    //grabber.stop();
                    cvReleaseCapture(capture);
                }
            } catch (Exception e2) {
                printStream.println(e2);
                printStream.flush();
                System.out.println(e2);
            }
            printStream.println(e);
            printStream.flush();
            System.out.println(e);
            captureWorking = false;
        }
    }

    //Слежение за красной точкой
    private void imgTrack() {

        IplImage origImg = IplImage.create(cvGetSize(IplImg), 8, 0);
        origImg = IplImg.clone();
        if (origImg != null) {
            cvSetImageROI(origImg, cvRect(0, origImg.height() / 2, origImg.width(), origImg.height()));//Выделение интересующей области
            cvCvtColor(origImg, origImg, CV_BGR2HSV); //Конвертируем изображение в HSB
            int x = 0; //Координаты красной точки
            int y = 0;
            //Нахождение точки по пороговым значениям

            IplImage imgThreshold = cvCreateImage(cvGetSize(origImg), 8, 1);

            //cvInRangeS(origImg, min, max, imgThreshold); //Получение порога (RGB)
            /*
             // Нахождение в по двум диапазонам HSB
             IplImage imgThreshold_Upper = cvCreateImage(cvGetSize(origImg), 8, 1);
             IplImage imgThreshold_Lower = cvCreateImage(cvGetSize(origImg), 8, 1);
             cvInRangeS(origImg, cvScalar(0, 150, 100, 0), cvScalar(5, 255, 255, 0), imgThreshold_Upper);
             cvInRangeS(origImg, cvScalar(170, 150, 100, 0), cvScalar(179, 255, 255, 0), imgThreshold_Lower);
             //Более узкий спектр оттенков
             // cvInRangeS(origImg, cvScalar(0, 150, 130, 0), cvScalar(5, 255, 255, 0), imgThreshold_Upper);
             // cvInRangeS(origImg, cvScalar(175, 150, 130, 0), cvScalar(179, 255, 255, 0), imgThreshold_Lower);
            
             cvAddWeighted(imgThreshold_Lower, 1.0, imgThreshold_Upper, 1.0, 0.0, imgThreshold);
             //cvSmooth(imgThreshold, imgThreshold, CV_MEDIAN, 3, 0, 0, 0); //Медианная фильтрация с аппертурой 3
             */
            cvInRangeS(origImg, min, max, imgThreshold);
            opencv_imgproc.CvMoments moments = new opencv_imgproc.CvMoments();
            cvMoments(imgThreshold, moments, 1);
            //Определение координат
            x = (int) (moments.m10() / moments.m00()); //Сумма координат x-точек делить на количество точек 
            y = (int) (moments.m01() / moments.m00()); //Сумма координат y-точек делить на количество точек 
            if (x == 0 && y == 0) {
                y = height / 2;
                x = width / 2;
            }
            cvReleaseImage(imgThreshold); //Очистка изображения ОЧЕНЬ ВАЖНО!!! БЕЗ ЭТОГО УТЕЧКА ПАМЯТИ!
            //Для двухдиапазонного HSB:
            //cvReleaseImage(imgThreshold_Upper); //Очистка изображения ОЧЕНЬ ВАЖНО!!! БЕЗ ЭТОГО УТЕЧКА ПАМЯТИ!
            //cvReleaseImage(imgThreshold_Lower); //Очистка изображения ОЧЕНЬ ВАЖНО!!! БЕЗ ЭТОГО УТЕЧКА ПАМЯТИ!
            imgThreshold = null;
            moments = null;
            SAU.filter(x, y);
            // инициализация шрифта
            CvFont font = new CvFont();
            cvInitFont(font, CV_FONT_HERSHEY_COMPLEX, 0.4, 0.8, 0, 1, CV_AA);
            // используя шрифт выводим на картинку текст
            cvPutText(IplImg, "Delay: " + timeDelay + " ms", cvPoint(5, 15), font, CV_RGB(255, 255, 255));
            double rv = (y - height / 2) * (y - height / 2) + (x - width / 2) * (x - width / 2);
            if (rv > 4) { //Если цель найдена
                CvPoint pt = cvPoint(SAU.getX(), SAU.getY() + height / 2);
                /*( IplImg.,// Изображение
                 pt, // Центр окружности
                 15, // Радиус
                 min, // Цвет
                 1, // Толщина
                 8 // Сглаживание
                 0, //Что-то ещё
                 );*/
                cvCircle(IplImg, pt, 30, CV_RGB(0, 255, 0), 3, 8, 0);
                pt = null;
            }
            font.deallocate();
            font = null;
            if (driveToPoint) //Если не работает настройка определения дальности
            {
                SAU.goToPoint();
            }
        }
        origImg.release();
        origImg = null;
    }

    public void stopVideo() { //Остановка видео
        captureWorking = false;

        try {
            this.join();
        } catch (Exception e) {
            printStream.println(e);
            printStream.flush();
            System.out.println(e);
        }
        if (SAU != null) {
            SAU.reset();
            SAU = null;
        }

    }

//Включение/выключение трансляции видео
    public void turnBroadcasting(boolean flag) {
        broadcastWorking = flag;
        //Остановка потока, если он не выполняет никаких функций
        if (!(broadcastWorking || trackingWorking)) {
            stopVideo();
        }
    }

//Включение/выключение отслеживания точки
    public void turnTracking(boolean flag) {
        trackingWorking = flag;
        if (trackingWorking && SAU == null) {
            SAU = new AutoSystem(width, height / 2);
        }
        if (SAU != null) {
            SAU.updateFilter();
        }
        if (!(broadcastWorking || trackingWorking || adjustWorking)) {
            stopVideo();
        }
        if (!(trackingWorking || adjustWorking || SAU == null)) {
            try {
                sleep(Tank_server.td);
            } catch (Exception e) {
                printStream.println(e);
                printStream.flush();
                System.out.println(e);
            } finally {
                SAU.reset();
            }
        }
    }

    //Включение/выключение настройки измерения дальности
    public void turnAdjust(boolean flag) {
        adjustWorking = flag;
        if (adjustWorking) {
            adjustCount = 0;
            adjustXSumm = 0;
            adjustYSumm = 0;
            if (SAU == null) {
                SAU = new AutoSystem(width, height / 2);
            }
            SAU.updateFilter();
        } else {
            //Определение фокусного расстояния и прочего
            double u = adjustXSumm / adjustCount;
            double v = adjustYSumm / adjustCount;
            double Zk = v * adjustDist; //Коэффициент для нахождения расстояния по вертикальной координате
            //Z=Zk/v
            double Xk = adjustRight / (u - width / 2) * v; //Коэффициент для нахождения угла до точки
            String str = "";
            try {
                File jarpath = new File(Tank_server.class.getProtectionDomain().getCodeSource().getLocation().toURI());
                File koefFile = new File(jarpath.getParent() + "/koef.ini");
                FileWriter saveFile = new FileWriter(koefFile);
                saveFile.write(String.valueOf(Zk) + '\n' + String.valueOf(Xk) + '\n');
                saveFile.close();
                str = "Adjust has been finished";
            } catch (Exception e) {
                str = e.toString();
            } finally {
                printStream.println(str);
                printStream.flush();
                System.out.println(str);
                if (!(broadcastWorking || trackingWorking || adjustWorking)) {
                    stopVideo();
                }
            }
            if (!(trackingWorking || adjustWorking || SAU == null)) {
                SAU.reset();
            }
        }
    }

    public void setDriveToPoint(boolean flag) {
        driveToPoint = flag;
    }

    public void setMinMaxHSB(CvScalar HSBmin, CvScalar HSBmax) {
        min = HSBmin;
        max = HSBmax;
        if (SAU != null) {
            SAU.updateFilter();
        }
    }

    //Транслируется ли видео
    public boolean isBroadcasting() {
        return broadcastWorking;
    }

//Отслеживается ли точка
    public boolean isTracking() {
        return trackingWorking;
    }

    //Настраивается ли определение расстояния
    public boolean isAdjusting() {
        return adjustWorking;
    }

//Захватывается ли виедо с камеры
    public boolean isWorking() {
        return captureWorking;
    }

    //Захватывается ли виедо с камеры
    public boolean isDriving() {
        return driveToPoint;
    }

    //Текущее время обработки
    public long getDelay() {
        return timeDelay;
    }
}
