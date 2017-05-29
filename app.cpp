/**
 ******************************************************************************
 ** ファイル名 : app.cpp
 **
 ** 概要 : 2輪倒立振子ライントレースロボットのTOPPERS/HRP2用C++サンプルプログラム
 **
 ** 注記 : sample_cpp (ライントレース/尻尾モータ/超音波センサ/リモートスタート)
 ******************************************************************************
 **/

#include "ev3api.h"
#include "app.h"
#include "balancer.h"
#include "TouchSensor.h"
#include "SonarSensor.h"
#include "ColorSensor.h"
#include "GyroSensor.h"
#include "Motor.h"
#include "Clock.h"
#include "Monitor.h"
#include "Music.h"
#include <string.h>

using namespace ev3api;

#define DEBUG

#ifdef DEBUG
#define _debug(x) (x)
#else
#define _debug(x)
#endif

/* Bluetooth */
static int32_t   bt_cmd = 0;      /* Bluetoothコマンド 1:リモートスタート */
static FILE     *bt = NULL;      /* Bluetoothファイルハンドル */

/* 下記のマクロは個体/環境に合わせて変更する必要があります */
#define GYRO_OFFSET           0  /* ジャイロセンサオフセット値(角速度0[deg/sec]時) */
#define LIGHT_WHITE          40  /* 白色の光センサ値 */
#define LIGHT_BLACK           0  /* 黒色の光センサ値 */
#define SONAR_ALERT_DISTANCE 30  /* 超音波センサによる障害物検知距離[cm] */
#define TAIL_ANGLE_STAND_UP  90  /* 完全停止時の角度[度] */
#define TAIL_ANGLE_DRIVE      3  /* バランス走行時の角度[度] */
#define P_GAIN             2.5F  /* 完全停止用モータ制御比例係数 */
#define PWM_ABS_MAX          60  /* 完全停止用モータ制御PWM絶対最大値 */
#define CMD_START         '1'    /* リモートスタートコマンド */

/* 関数プロトタイプ宣言 */
static int32_t sonar_alert(void);
static void tail_control(int32_t angle);

/* オブジェクトへのポインタ定義 */
TouchSensor*    touchSensor;
SonarSensor*    sonarSensor;
ColorSensor*    colorSensor;
GyroSensor*     gyroSensor;
Motor*          leftMotor;
Motor*          rightMotor;
Motor*          tailMotor;
Clock*          clock;
Monitor*        monitor;
Music*          music;

/* メインタスク */
void main_task(intptr_t unused)
{
    int8_t forward;      /* 前後進命令 */
    int8_t turn;         /* 旋回命令 */
    int8_t pwm_L, pwm_R; /* 左右モータPWM出力 */

    /* 各オブジェクトを生成・初期化する */
    touchSensor = new TouchSensor(PORT_1);
    sonarSensor = new SonarSensor(PORT_2);
    colorSensor = new ColorSensor(PORT_3);
    gyroSensor  = new GyroSensor(PORT_4);
    leftMotor   = new Motor(PORT_C);
    rightMotor  = new Motor(PORT_B);
    tailMotor   = new Motor(PORT_A);
    clock       = new Clock();
    monitor     = new Monitor();
    music       = new Music((uint8_t)1);

    char        buf[256];

    monitor->display("start running-test");

    // sprintf(buf,"now1 is %-u",(uint32_t)clock->now());
    // monitor->display(buf);

    // monitor->display("play music");
    // music->playMusic();
    // monitor->display("play music end");

    // sprintf(buf,"now2 is %-u",(uint32_t)clock->now());
    // monitor->display(buf);
    // clock->reset();
    // sprintf(buf,"now3 is %-u",(uint32_t)clock->now());
    // monitor->display(buf);

    /* 尻尾モーターのリセット */
    tailMotor->reset();

    /* Open Bluetooth file */
    bt = ev3_serial_open_file(EV3_SERIAL_BT);
    assert(bt != NULL);

    /* Bluetooth通信タスクの起動 */
    act_tsk(BT_TASK);

    ev3_led_set_color(LED_ORANGE); /* 初期化完了通知 */

    /* スタート待機 */
    while(1)
    {
        tail_control(TAIL_ANGLE_STAND_UP); /* 完全停止用角度に制御 */

        if (bt_cmd == 1)
        {
            break; /* リモートスタート */
        }

        if (touchSensor->isPressed())
        {
            break; /* タッチセンサが押された */
        }

        clock->sleep(10);
    }

    /* 走行モーターエンコーダーリセット */
    leftMotor->reset();
    rightMotor->reset();

    /* ジャイロセンサーリセット */
    gyroSensor->reset();
    balance_init(); /* 倒立振子API初期化 */

    ev3_led_set_color(LED_GREEN); /* スタート通知 */
    monitor->display("touch now");

    /**
    * Main loop for the self-balance control algorithm
    */
    // char buf[256];
    int32_t motor_ang_l, motor_ang_r;
    int32_t gyro, volt;

    music->playTone();


    monitor->display("forward");

    // 前進
    clock->reset();
    while(10000 >= clock->now())
    {
        // sprintf(buf,"now is %-u",(uint32_t)clock->now());
        // monitor->display(buf);

        // /** 反射光の強さ **/
        // int8_t brightness = colorSensor->getBrightness();
        // sprintf(buf,"Brightness is %-d",brightness);
        // monitor->display(buf);
        // memset(buf, '\0', strlen(buf));

        if (ev3_button_is_pressed(BACK_BUTTON)) break;

        tail_control(TAIL_ANGLE_DRIVE); /* バランス走行用角度に制御 */
        // forward = 30;   /* 前進命令 */
        forward = 60;   /* 前進命令 */
        turn = 0;       /* 旋回命令 */
        /* 倒立振子制御API に渡すパラメータを取得する */
        motor_ang_l = leftMotor->getCount();
        motor_ang_r = rightMotor->getCount();
        gyro = gyroSensor->getAnglerVelocity();
        volt = ev3_battery_voltage_mV();

        /* 倒立振子制御APIを呼び出し、倒立走行するための */
        /* 左右モータ出力値を得る */
        balance_control(
            (float)forward,
            (float)turn,
            (float)gyro,
            (float)GYRO_OFFSET,
            (float)motor_ang_l,
            (float)motor_ang_r,
            (float)volt,
            (int8_t *)&pwm_L,
            (int8_t *)&pwm_R);

        leftMotor->setPWM(pwm_L);
        rightMotor->setPWM(pwm_R);

        clock->sleep(4); /* 4msec周期起動 */
    }

    monitor->display("turn");

    clock->reset();
    // 旋回
    while(1900 >= clock->now())
    {
        // /** 反射光の強さ **/
        // int8_t brightness = colorSensor->getBrightness();
        // sprintf(buf,"Brightness is %-d",brightness);
        // monitor->display(buf);
        // memset(buf, '\0', strlen(buf));
        if (ev3_button_is_pressed(BACK_BUTTON)) break;

        tail_control(TAIL_ANGLE_DRIVE); /* バランス走行用角度に制御 */
        forward = 0;   /* 前進命令 */
        turn = 30;       /* 旋回命令 */
        /* 倒立振子制御API に渡すパラメータを取得する */
        motor_ang_l = leftMotor->getCount();
        motor_ang_r = rightMotor->getCount();
        gyro = gyroSensor->getAnglerVelocity();
        volt = ev3_battery_voltage_mV();

        /* 倒立振子制御APIを呼び出し、倒立走行するための */
        /* 左右モータ出力値を得る */
        balance_control(
            (float)forward,
            (float)turn,
            (float)gyro,
            (float)GYRO_OFFSET,
            (float)motor_ang_l,
            (float)motor_ang_r,
            (float)volt,
            (int8_t *)&pwm_L,
            (int8_t *)&pwm_R);

        leftMotor->setPWM(pwm_L);
        rightMotor->setPWM(pwm_R);

        clock->sleep(4); /* 4msec周期起動 */
    }

    monitor->display("speed up");
    clock->reset();
    // 加速前進
    while(3000 >= clock->now())
    {
        // /** 反射光の強さ **/
        // int8_t brightness = colorSensor->getBrightness();
        // sprintf(buf,"Brightness is %-d",brightness);
        // monitor->display(buf);
        // memset(buf, '\0', strlen(buf));

        if (ev3_button_is_pressed(BACK_BUTTON)) break;

        tail_control(TAIL_ANGLE_DRIVE); /* バランス走行用角度に制御 */
        // forward = 90;   /* 前進命令 */
        forward = 100;   /* 前進命令 */
        turn = 0;       /* 旋回命令 */
        /* 倒立振子制御API に渡すパラメータを取得する */
        motor_ang_l = leftMotor->getCount();
        motor_ang_r = rightMotor->getCount();
        gyro = gyroSensor->getAnglerVelocity();
        volt = ev3_battery_voltage_mV();

        /* 倒立振子制御APIを呼び出し、倒立走行するための */
        /* 左右モータ出力値を得る */
        balance_control(
            (float)forward,
            (float)turn,
            (float)gyro,
            (float)GYRO_OFFSET,
            (float)motor_ang_l,
            (float)motor_ang_r,
            (float)volt,
            (int8_t *)&pwm_L,
            (int8_t *)&pwm_R);

        leftMotor->setPWM(pwm_L);
        rightMotor->setPWM(pwm_R);

        clock->sleep(4); /* 4msec周期起動 */
    }

    monitor->display("stop");

    clock->reset();
    // 静止
    while(1000 >= clock->now())
    {
        // /** 反射光の強さ **/
        // int8_t brightness = colorSensor->getBrightness();
        // sprintf(buf,"Brightness is %-d",brightness);
        // monitor->display(buf);
        // memset(buf, '\0', strlen(buf));

        if (ev3_button_is_pressed(BACK_BUTTON)) break;

        tail_control(TAIL_ANGLE_DRIVE); /* バランス走行用角度に制御 */
        forward = 0;
        turn = 0;
        /* 倒立振子制御API に渡すパラメータを取得する */
        motor_ang_l = leftMotor->getCount();
        motor_ang_r = rightMotor->getCount();
        gyro = gyroSensor->getAnglerVelocity();
        volt = ev3_battery_voltage_mV();

        /* 倒立振子制御APIを呼び出し、倒立走行するための */
        /* 左右モータ出力値を得る */
        balance_control(
            (float)forward,
            (float)turn,
            (float)gyro,
            (float)GYRO_OFFSET,
            (float)motor_ang_l,
            (float)motor_ang_r,
            (float)volt,
            (int8_t *)&pwm_L,
            (int8_t *)&pwm_R);

        leftMotor->setPWM(pwm_L);
        rightMotor->setPWM(pwm_R);

        clock->sleep(4); /* 4msec周期起動 */
    }

    clock->reset();
    // 停止
    while(1)
    {
        // /** 反射光の強さ **/
        // int8_t brightness = colorSensor->getBrightness();
        // sprintf(buf,"Brightness is %-d",brightness);
        // monitor->display(buf);
        // memset(buf, '\0', strlen(buf));

        if (ev3_button_is_pressed(BACK_BUTTON)) break;

        tail_control(85); /* バランス走行用角度に制御 */

        forward = 0;
        turn = 0;

        /* 倒立振子制御API に渡すパラメータを取得する */
        motor_ang_l = leftMotor->getCount();
        motor_ang_r = rightMotor->getCount();
        gyro = gyroSensor->getAnglerVelocity();
        volt = ev3_battery_voltage_mV();

        /* 倒立振子制御APIを呼び出し、倒立走行するための */
        /* 左右モータ出力値を得る */
        balance_control(
            (float)forward,
            (float)turn,
            (float)gyro,
            (float)GYRO_OFFSET,
            (float)motor_ang_l,
            (float)motor_ang_r,
            (float)volt,
            (int8_t *)&pwm_L,
            (int8_t *)&pwm_R);

        leftMotor->setPWM(pwm_L);
        rightMotor->setPWM(pwm_R);

        clock->sleep(4); /* 4msec周期起動 */

        if(90 < tailMotor->getCount())
        {
            break;
        }
    }

    clock->reset();
    // 静止
    while(1000 >= clock->now())
    {
        leftMotor->setPWM(5);
        rightMotor->setPWM(5);
    }

    music->playTone();

    leftMotor->reset();
    rightMotor->reset();
    tailMotor->reset();

    ter_tsk(BT_TASK);
    fclose(bt);

    monitor->display("end running-test");
    delete monitor;

    ext_tsk();
}

//*****************************************************************************
// 関数名 : sonar_alert
// 引数 : 無し
// 返り値 : 1(障害物あり)/0(障害物無し)
// 概要 : 超音波センサによる障害物検知
//*****************************************************************************
static int32_t sonar_alert(void)
{
    static uint32_t counter = 0;
    static int32_t alert = 0;

    int32_t distance;

    if (++counter == 40/4) /* 約40msec周期毎に障害物検知  */
    {
        /*
         * 超音波センサによる距離測定周期は、超音波の減衰特性に依存します。
         * NXTの場合は、40msec周期程度が経験上の最短測定周期です。
         * EV3の場合は、要確認
         */
        distance = sonarSensor->getDistance();
        if ((distance <= SONAR_ALERT_DISTANCE) && (distance >= 0))
        {
            alert = 1; /* 障害物を検知 */
        }
        else
        {
            alert = 0; /* 障害物無し */
        }
        counter = 0;
    }

    return alert;
}

//*****************************************************************************
// 関数名 : tail_control
// 引数 : angle (モータ目標角度[度])
// 返り値 : 無し
// 概要 : 走行体完全停止用モータの角度制御
//*****************************************************************************
static void tail_control(int32_t angle)
{
    // float pwm = (float)(angle - tailMotor->getCount()) * P_GAIN; /* 比例制御 */
    float pwm = (float)(angle - tailMotor->getCount()) * 0.8; /* 比例制御 */
    /* PWM出力飽和処理 */
    if (pwm > PWM_ABS_MAX)
    {
        pwm = PWM_ABS_MAX;
    }
    else if (pwm < -PWM_ABS_MAX)
    {
        pwm = -PWM_ABS_MAX;
    }

    tailMotor->setPWM(pwm);
}

//*****************************************************************************
// 関数名 : bt_task
// 引数 : unused
// 返り値 : なし
// 概要 : Bluetooth通信によるリモートスタート。 Tera Termなどのターミナルソフトから、
//       ASCIIコードで1を送信すると、リモートスタートする。
//*****************************************************************************
void bt_task(intptr_t unused)
{
    while(1)
    {
        uint8_t c = fgetc(bt); /* 受信 */
        switch(c)
        {
        case '1':
            bt_cmd = 1;
            break;
        default:
            break;
        }
        fputc(c, bt); /* エコーバック */
    }
}
