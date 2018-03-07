/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package tank_server;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import static java.lang.Math.PI;
import static java.lang.Math.atan;
import static java.lang.Math.sqrt;

/**
 *
 * @author alexey
 */
public class AutoSystem {

    private int width = 640; //Максимальное значение горизонтальной координаты
    private int height = 240; //Максимальное значение вертикальной координаты
    //Предыдущие реакции и воздействия на фильтр
    private final int[] Ux = {0, 0, 0};
    private final double[] Vx = {0, 0, 0};
    private final int[] Uy = {0, 0, 0};
    private final double[] Vy = {0, 0, 0};

    //Предыдущие реакции и воздействия на интегратор
    private final double[] UI = {0, 0};
    private final double[] VI = {0, 0};

    private double Td = (double) (Tank_server.td) / 1000;   //Время дискретизации в секундах

    //Коэффициенты аналогового эквивалента фильтра
    private final double[] bs = {0, 0, 400};
    private final double[] as = {1, 28.284271247461900, 400};

    //Коэффициенты цифрового фильтра
    private final double[] b = new double[3];
    private final double[] a = new double[3];

    //Коэффициенты для нахождения расстояний и углов
    double Zk = 46;
    double Xk = 0.0020408163278306124;

    //Конструктор
    public AutoSystem(int maxX, int maxY) {
        width = maxX;
        height = maxY;
        for (int i = 0; i <= 2; i++) { //Начальные значения фильтра
            Ux[i] = width / 2;
            Vx[i] = height;
            Uy[i] = width / 2;
            Vy[i] = height;
        }
        updateFilter();
        try {
            File jarpath = new File(Tank_server.class.getProtectionDomain().getCodeSource().getLocation().toURI());
            File koefFile = new File(jarpath.getParent() + "/koef.ini");
            if (koefFile.exists()) {
                FileReader fileStream = new FileReader(koefFile);
                BufferedReader reader = new BufferedReader(fileStream);
                Zk = Double.parseDouble(reader.readLine());
                Xk = Double.parseDouble(reader.readLine());
            }
        } catch (Exception e) {
            System.out.println(e);
        }
    }

//Фильтр Баттерворта второго порядка с частотой среза 20 рад/с
    public void filter(int x, int y) {

        Ux[2] = Ux[1];
        Uy[2] = Uy[1];
        Ux[1] = Ux[0];
        Uy[1] = Uy[0];
        Ux[0] = x;
        Uy[0] = y;
        Vx[2] = Vx[1];
        Vy[2] = Vy[1];
        Vx[1] = Vx[0];
        Vy[1] = Vy[0];
        Vx[0] = b[0] * Ux[0] + b[1] * Ux[1] + b[2] * Ux[2] - a[1] * Vx[1] - a[2] * Vx[2];
        Vy[0] = b[0] * Uy[0] + b[1] * Uy[1] + b[2] * Uy[2] - a[1] * Vy[1] - a[2] * Vy[2];
    }

    //Возвращение отфильтрованных координат.
    public int getX() {
        if (Vx[0] >= 0) {
            return (int) Vx[0];
        } else {
            return 0;
        }
    }

    public int getY() {
        if (Vy[0] >= 0) {
            return (int) Vy[0];
        } else {
            return 0;
        }
    }

    //САУ
    public void goToPoint(double x, double y) {
        double Z = Zg() + 0.15;
        double X = Xg();
        double D = sqrt(Z * Z + X * X); //Дальность до цели
        double fi = atan(X / Z);

        //Выработка требуемых линейной и угловой скоростей
        double Kd = 2;
        double Kfi = 6.54;
        double v = (D - 0.45) * Kd;
        String command = "SAUoff:";
        if (v < 0.06) {
            v = 0;
        } else {
            double w = fi * Kfi;
            String Vcommand = "v";
            Vcommand += String.valueOf(v) + ':';
            String Wcommand = "w";
            Wcommand += String.valueOf(w) + ':';
            command = Vcommand + Wcommand;
        }
        //.out.println("D=" + D + ", fi=" + fi * 180 / PI);
        try {
            Tank_server.serialPort.writeString(command); //Передача команды на Arduino
        } catch (Exception e) {
            System.out.println(e);
        } finally {
            command = null;
        }

    }

    public void goToPoint() {
        goToPoint(Vx[0], Vy[0]);
    }

    
    //Фотограмметрия
    private double Zg() {
        double v = Vy[0];
        double Z;
        if (v == 0) {
            Z = 0;
        } else {
            Z = Zk / v; //Проекция расстояния от камеры до цели на опт. ось
        }
        return Z;
    }

    private double Xg() {
        double u = Vx[0];
        double v = Vy[0];
        double X;
        //System.out.println(Xk);
        X = Xk * (u - width / 2) / v;
        return X;
    }

//Сброс истории воздействий и рекций
    public void reset() {
        for (int i = 0; i <= 2; i++) {
            Ux[i] = width / 2;
            Vx[i] = height;
            Uy[i] = width / 2;
            Vy[i] = height;
            if (i <= 1) {
                VI[i] = 0;
                UI[i] = 0;
            }
        }
        try {
            Tank_server.serialPort.writeString("R0:L0:"); //Передача команды на Arduino
        } catch (Exception e) {
            System.out.println(e);
        }
    }

    //Изменение параметров при изменении времени дискретизации
    public final void updateFilter() {
        Td = (double) (Tank_server.td) / 1000;
        double u = 4 / (Td * Td) + 2 * as[1] / Td + as[2];
        b[0] = bs[2] / u;
        b[1] = 2 * bs[2] / u;
        b[2] = bs[2] / u;
        a[0] = 1;
        a[1] = (-8 / (Td * Td) + 2 * as[2]) / u;
        a[2] = (4 / (Td * Td) - 2 * as[1] / Td + as[2]) / u;
    }
}
