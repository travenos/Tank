/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package tank_server;

import java.net.ServerSocket;
import java.net.Socket;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.PrintStream;
import java.util.ArrayList;
import jssc.SerialPort;
import java.net.InetAddress;
import static org.bytedeco.javacpp.opencv_core.*;

/**
 *
 * @author alexey
 */
public class Tank_server {

    /**
     * @param args the command line arguments
     */
    public static int td = 90; //Время дискретизации в милисекундах
    public static SerialPort serialPort = null; //COM-порт

    private static ServerSocket serverSocket = null;
    private static boolean connected = false; //Клиент подключён
    private static PrintStream ps = null;  //Поток отправки клиенту
    private static String tty = "/dev/ttyUSB0";   //Обозначение Arduino в системе
    private final static int TCPport = 8888; //Порт TCP-сервера
    private static InetAddress firstClientAddr = null;
    private static boolean lostConnectionStop = true;

    static VideoProcess video = null;
    private static CvScalar cvScmin = cvScalar(170, 150, 100, 0);   //Минимальная интенсивность точки в HSB
    private static CvScalar cvScmax = cvScalar(179, 255, 255, 0);   //Максимальная интенсивность точки в HSB

    public static void main(String[] args) {
        try {
            serverSocket = new ServerSocket(TCPport); //Серверный сокет
            System.out.println("Server started");
            //Считывание настроек
            try {
                File jarpath = new File(Tank_server.class.getProtectionDomain().getCodeSource().getLocation().toURI());
                File paramFile = new File(jarpath.getParent() + "/param.ini");
                FileReader fileStream = new FileReader(paramFile);
                BufferedReader reader = new BufferedReader(fileStream);
                cvScmin = cvScalar(Integer.parseInt(reader.readLine()), Integer.parseInt(reader.readLine()), Integer.parseInt(reader.readLine()), 0);   //Минимальная интенсивность точки в HSB
                cvScmax = cvScalar(Integer.parseInt(reader.readLine()), Integer.parseInt(reader.readLine()), Integer.parseInt(reader.readLine()), 0);   //Максимальная интенсивность точки в HSB
                td = Integer.parseInt(reader.readLine());
                reader.close();
                fileStream.close();
            } catch (Exception e) {
                System.out.println(e);
            }
            //Открытие COM-порта
            boolean com_on = comport(true);
            if (!com_on) {
                System.out.println("Unable to open COM-port");
            }
            //Цикл ожидания подключения
            while (true) {
                Socket clientSocket = serverSocket.accept(); //Ожидание клиента
                final PrintStream printStr = new PrintStream(clientSocket.getOutputStream()); //Поток отправки клиенту
                if (!connected) {
                    firstClientAddr = clientSocket.getInetAddress();
                    InputStream in = clientSocket.getInputStream(); //Получение потока ввода
                    BufferedReader reader = new BufferedReader(new InputStreamReader(in));

                    if (!com_on) {
                        printStr.println("Unable to open COM-port");
                        printStr.flush();
                    }
                    Thread myThready = new Thread(new Runnable() {
                        public void run() //Дешифрация команд - в отдельном потоке
                        {
                            analizer(reader, printStr); //Постоянное чтение потока
                        }
                    });
                    myThready.start();	//Запуск потока
                } else { //Если уже кто-то подключён
                    printStr.println("Only one connection is available");
                    printStr.close();
                    clientSocket.close();
                    clientSocket = null;
                }
            }

        } catch (Exception e) {
            System.out.println(e);
            restart();
        }

    }

    //Постоянное чтение потока
    private static void analizer(BufferedReader rd, PrintStream p) {
        BufferedReader reader = rd;
        ps = p;
        connected = true;
        try {
            String command;
            while ((command = reader.readLine()) != null) {
                System.out.println(command);
                System.out.flush();
                try {
                    //Выполнение программы в системе по ключевому слову "linux"
                    //Отключено в целях безопасности ПО на роботе
                    /*
                     if (command.startsWith("linux"))
                     {
                     String[] lol;
                     lol = command.substring(6).split(" ");
                     ProcessBuilder builder = new ProcessBuilder(lol);
                     builder.start();
                     }                         
                     else
                     */
                    //Установка параметров видео
                    if (command.startsWith("PARAM")) {
                        String[] lol;
                        lol = command.substring(6).split(" ");
                        cvScmin = cvScalar(Integer.parseInt(lol[0]), Integer.parseInt(lol[2]), Integer.parseInt(lol[4]), 0);   //Минимальная интенсивность точки в HSB
                        cvScmax = cvScalar(Integer.parseInt(lol[1]), Integer.parseInt(lol[3]), Integer.parseInt(lol[5]), 0);   //Максимальная интенсивность точки в HSB
                        td = Integer.parseInt(lol[6]);
                        saveParam();
                        lol = null;
                        if (video != null) {
                            video.setMinMaxHSB(cvScmin, cvScmax);
                        }
                    } else if (command.startsWith("tty")) { //Смена обозначения Arduino в системе по ключевому слову "tty"
                        String[] lol;
                        lol = command.split(" ");
                        tty = lol[1];
                        if (!comport(true)) {
                            ps.println("Unable to open COM-port");
                            ps.flush();
                        }
                        lol = null;
                    } else {
                        switch (command.intern()) {
                            case "exit":        //Выход из программы
                                if (video != null) {
                                    video.stopVideo();
                                    video = null;
                                }
                                try {
                                    serialPort.writeString("R0:L0:SAUoff:");
                                } catch (Exception e) {
                                    System.out.println(e);
                                    ps.println(e);
                                    ps.flush();
                                } finally {
                                    saveParam();
                                    System.exit(0);
                                }
                                break;
                            case "restart":        //Перезапуск программы
                                saveParam();
                                restart();
                                break;
                            case "reset": //Перезапуск компьютера
                            {
                                try {
                                    serialPort.writeString("R0:L0:SAUoff:");
                                } catch (Exception e) {
                                    System.out.println(e);
                                    ps.println(e);
                                    ps.flush();
                                } finally {
                                    saveParam();
                                    ArrayList<String> cons_command = new ArrayList<String>(); //Комманда для выполнения в консоли
                                    cons_command.add("shutdown");
                                    cons_command.add("-r");
                                    cons_command.add("now");
                                    ProcessBuilder builder = new ProcessBuilder(cons_command);
                                    builder.start();
                                }
                                break;
                            }
                            case "shutdown": //Выключение компьютера
                            {
                                try {
                                    serialPort.writeString("R0:L0:SAUoff:");
                                } catch (Exception e) {
                                    System.out.println(e);
                                    ps.println(e);
                                    ps.flush();
                                } finally {
                                    saveParam();
                                    ArrayList<String> cons_command = new ArrayList<String>(); //Комманда для выполнения в консоли
                                    cons_command.add("shutdown");
                                    cons_command.add("-h");
                                    cons_command.add("now");
                                    ProcessBuilder builder = new ProcessBuilder(cons_command);
                                    builder.start();
                                }
                                break;
                            }
                            case "video_on": { //Включение трансляции видео
                                if (video != null) {
                                    if (!video.isWorking()) {
                                        video = null;
                                    }
                                }
                                if (video == null) {
                                    video = new VideoProcess(ps, firstClientAddr);
                                    video.setMinMaxHSB(cvScmin, cvScmax);
                                }
                                video.turnBroadcasting(true);
                                if (!video.isWorking()) {
                                    video.start();
                                }
                                break;
                            }
                            case "video_off": { //Выключение трансляции видео
                                if (video != null) {
                                    video.turnBroadcasting(false);
                                    //video.stopVideo();
                                    if (!video.isWorking()) {
                                        video = null;
                                    }
                                }
                                break;
                            }
                            case "ff_on": {  //Включение FF-сервера
                                ArrayList<String> cons_command = new ArrayList<String>(); //Комманда для выполнения в консоли
                                cons_command.add("/bin/bash");
                                cons_command.add("/opt/ff_start.sh");
                                ProcessBuilder builder = new ProcessBuilder(cons_command);
                                builder.start();
                                break;
                            }
                            case "ff_off": { //Выключение FF-сервера
                                ArrayList<String> cons_command = new ArrayList<String>(); //Комманда для выполнения в консоли
                                cons_command.add("killall");
                                cons_command.add("ffmpeg");
                                cons_command.add("ffserver");
                                ProcessBuilder builder = new ProcessBuilder(cons_command);
                                builder.start();
                                break;
                            }
                            case "tracking_on": {  //Включение слежения за точкой
                                tracking(true);
                                video.setDriveToPoint(true);
                                break;
                            }
                            case "tracking_off": {
                                tracking(false);
                                break;
                            }
                            case "adjust_on": { //Настройка коэффициентов для определения координат
                                String str = "Put a red object at distance of 1m straight and 20 cm right in front of the camera";
                                System.out.println(str);
                                ps.println(str);
                                ps.flush();
                                if (video != null) {
                                    if (!video.isWorking()) {
                                        video = null;
                                    }
                                }
                                if (video == null) {
                                    video = new VideoProcess(ps, firstClientAddr);
                                    video.setMinMaxHSB(cvScmin, cvScmax);
                                }
                                video.turnAdjust(true);
                                video.setDriveToPoint(false);
                                if (!video.isWorking()) {
                                    video.start();
                                }
                                break;
                            }
                            case "adjust_off": {
                                if (video != null) {
                                    video.turnAdjust(false);
                                    video.setDriveToPoint(false);
                                    //video.stopVideo();
                                    if (!video.isWorking()) {
                                        video = null;
                                    }
                                }
                                break;
                            }
                            case "getTD": {
                                if (video != null) {
                                    ps.println("Current delay is " + video.getDelay());
                                } else {
                                    ps.println("Camera is off");
                                }
                                ps.flush();
                                break;
                            }
                            case "settings_on": {
                                tracking(true);
                                video.turnBroadcasting(true);
                                video.setDriveToPoint(false);
                                break;
                            }
                            case "settings_off": {
                                if (video != null) {
                                    video.setDriveToPoint(true);
                                    tracking(false);
                                }
                                break;
                            }
                            case "autonom_on": {
                                lostConnectionStop = false;
                                break;
                            }
                            case "autonom_off": {
                                lostConnectionStop = true;
                                break;
                            }
                            case "getParam": {
                                String answ = "PARAM ";
                                answ += String.valueOf((int) cvScmin.val(0));
                                answ += ' ';
                                answ += String.valueOf((int) cvScmax.val(0));
                                answ += ' ';
                                answ += String.valueOf((int) cvScmin.val(1));
                                answ += ' ';
                                answ += String.valueOf((int) cvScmax.val(1));
                                answ += ' ';
                                answ += String.valueOf((int) cvScmin.val(2));
                                answ += ' ';
                                answ += String.valueOf((int) cvScmax.val(2));
                                answ += ' ';
                                answ += String.valueOf(td);
                                ps.println(answ);
                                ps.flush();
                                answ = null;
                                break;
                            }
                            default:
                                serialPort.writeString(command); //Передача команды на Arduino
                                break;
                        }
                    }
                } catch (Exception e) {
                    System.out.println(e);
                    ps.println(e);
                    ps.flush();
                }
            }
        } catch (Exception e) {
            System.out.println(e);
            ps.println(e);
            ps.flush();
        } finally {
            connected = false;
            if (video != null) {
                if (lostConnectionStop) {
                    video.stopVideo();
                    video = null;
                } else {
                    video.turnBroadcasting(false);
                    if (video.isAdjusting()) {
                        video.turnAdjust(false);
                    }
                }
            }
            System.gc(); //Запрос на сборку мусора
        }
    }

    //Перезапуск программы
    private static void restart() {
        try {
            serialPort.writeString("L0:R0:SAUoff:");
        } catch (Exception e) {
            System.out.println(e);
        } finally {
            try {
                if (serverSocket != null) {
                    serverSocket.close();
                }
                if (video != null) {
                    video.stopVideo();
                    video = null;
                }
                File jarpath = new File(Tank_server.class.getProtectionDomain().getCodeSource().getLocation().toURI());
                ArrayList<String> cons_command = new ArrayList<String>(); //Комманда для выполнения в консоли
                cons_command.add("java");
                cons_command.add("-jar");
                cons_command.add(jarpath.getPath());
                ProcessBuilder builder = new ProcessBuilder(cons_command);
                builder.start();
                System.exit(0);
            } catch (Exception e) {
                System.out.println(e);
            }
        }
    }

    //Переключение слежения за точкой
    private static void tracking(boolean state) {
        if (state) {
            if (video != null) {
                if (!video.isWorking()) {
                    video = null;
                }
            }
            if (video == null) {
                video = new VideoProcess(ps, firstClientAddr);
                video.setMinMaxHSB(cvScmin, cvScmax);
            }
            video.turnTracking(true);
            if (!video.isWorking()) {
                video.start();
            }
        } else {
            if (video != null) {
                video.turnTracking(false);
                //video.stopVideo();
                if (!video.isWorking()) {
                    video = null;
                }
            }
        }
    }

    //Открытие COM-порта
    private static boolean comport(boolean state) {
        try {
            if (state) {
                if (serialPort != null) {
                    if (serialPort.isOpened()) {
                        serialPort.closePort();
                    }
                    serialPort = null;
                }
                serialPort = new SerialPort(tty);
                serialPort.openPort();
                serialPort.setParams(SerialPort.BAUDRATE_115200,
                        SerialPort.DATABITS_8,
                        SerialPort.STOPBITS_1,
                        SerialPort.PARITY_NONE);
                //Включаем аппаратное управление потоком
                serialPort.setFlowControlMode(SerialPort.FLOWCONTROL_RTSCTS_IN
                        | SerialPort.FLOWCONTROL_RTSCTS_OUT);
            } else {
                if (serialPort != null) {
                    if (serialPort.isOpened()) {
                        serialPort.closePort();
                    }
                    serialPort = null;
                }
            }
            return true;
        } catch (Exception e) {
            System.out.println(e);
            return false;
        }
    }

    //Сохранение параметров в файл
    private static void saveParam() {
        try {
            File jarpath = new File(Tank_server.class.getProtectionDomain().getCodeSource().getLocation().toURI());
            File paramFile = new File(jarpath.getParent() + "/param.ini");
            FileWriter saveFile = new FileWriter(paramFile);
            for (int i = 0; i <= 2; i++) {
                saveFile.write(String.valueOf((int) cvScmin.get(i)) + "\n");
            }
            for (int i = 0; i <= 2; i++) {
                saveFile.write(String.valueOf((int) cvScmax.get(i)) + "\n");
            }
            saveFile.write(String.valueOf(td) + "\n");
            saveFile.close();
        } catch (Exception e) {
            System.out.println(e);
        }
    }
}
