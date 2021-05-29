#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <DNSServer.h>//密码直连将其三个库注释
#include <ESP8266WebServer.h>
#include <CustomWiFiManager.h>

#include <time.h>                       
#include <sys/time.h>                  
#include <coredecls.h>      


//#include "SH1106Wire.h"   //1.3寸用这个
#include "SSD1306Wire.h"    //0.96寸用这个
#include "OLEDDisplayUi.h"
#include "HeFeng.h"
#include "WeatherStationFonts.h"
#include "WeatherStationImages.h"

/***************************
   Begin Settings
 **************************/

const char* WIFI_SSID = "TP-LINK_XXXX";  //填写你的WIFI名称及密码
const char* WIFI_PWD = "123456789";

const char* BILIBILIID = "35549294";  //填写你的B站账号

//由于太多人使用我的秘钥，导致获取次数超额，所以不提供秘钥了，大家可以到https://dev.heweather.com/获取免费的
const char* HEFENG_KEY = "XXXXXXXXXXXXXXXXXXXX";//填写你的和风天气秘钥
const char* HEFENG_LOCATION = "101180109";//填写你的城市ID,可到https://where.heweather.com/index.html查询
//const char* HEFENG_LOCATION = "auto_ip";//自动IP定位

#define TZ              8      // 中国时区为8
#define DST_MN          0      // 默认为0

const int UPDATE_INTERVAL_SECS = 5 * 60; // 5分钟更新一次天气
const int UPDATE_CURR_INTERVAL_SECS = 2 * 59; // 2分钟更新一次粉丝数

const int I2C_DISPLAY_ADDRESS = 0x3c;  //I2c地址默认
#if defined(ESP8266)
const int SDA_PIN = 0;  //引脚连接
const int SDC_PIN = 2;  //
#endif

const String WDAY_NAMES[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};  //星期
const String MONTH_NAMES[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};  //月份

// web配网页面自定义我的图标请随便使用一个图片转base64工具转换https://tool.css-js.com/base64.html, 64*64
const char Icon[] PROGMEM = "data:image/png;base64,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";

/***************************
   End Settings
 **************************/
 
//SH1106Wire     display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);   // 1.3寸用这个
SSD1306Wire     display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);   // 0.96寸用这个
OLEDDisplayUi   ui( &display );

HeFengCurrentData currentWeather; //实例化对象
HeFengForeData foreWeather[3];
HeFeng HeFengClient;

#define TZ_MN           ((TZ)*60)   //时间换算
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)

time_t now; //实例化时间

bool readyForWeatherUpdate = false; // 天气更新标志
bool first = true;  //首次更新标志
long timeSinceLastWUpdate = 0;    //上次更新后的时间
long timeSinceLastCurrUpdate = 0;   //上次天气更新后的时间

String fans = "-1"; //粉丝数

void drawProgress(OLEDDisplay *display, int percentage, String label);   //提前声明函数
void updateData(OLEDDisplay *display);
void updateDatas(OLEDDisplay *display);
void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawForecast(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex);
void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state);
void setReadyForWeatherUpdate();

//添加框架
//此数组保留指向所有帧的函数指针
//框架是从右向左滑动的单个视图
FrameCallback frames[] = { drawDateTime, drawCurrentWeather, drawForecast };
//页面数量
int numberOfFrames = 3;

OverlayCallback overlays[] = { drawHeaderOverlay }; //覆盖回调函数
int numberOfOverlays = 1;  //覆盖数

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // 屏幕初始化
  display.init();
  display.clear();
  display.display();

  display.flipScreenVertically(); //屏幕翻转
  display.setContrast(255); //屏幕亮度

  //Web配网，密码直连请注释
//  webconnect();
  
  // 用固定密码连接，Web配网请注释
  wificonnect();

  ui.setTargetFPS(30);  //刷新频率

  ui.setActiveSymbol(activeSymbole); //设置活动符号
  ui.setInactiveSymbol(inactiveSymbole); //设置非活动符号

  // 符号位置
  // 你可以把这个改成TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);

  // 定义第一帧在栏中的位置
  ui.setIndicatorDirection(LEFT_RIGHT);

  // 屏幕切换方向
  // 您可以更改使用的屏幕切换方向 SLIDE_LEFT, SLIDE_RIGHT, SLIDE_TOP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  ui.setFrames(frames, numberOfFrames); // 设置框架
  ui.setTimePerFrame(5000); //设置切换时间
  
  ui.setOverlays(overlays, numberOfOverlays); //设置覆盖

  // UI负责初始化显示
  ui.init();
  display.flipScreenVertically(); //屏幕反转

  configTime(TZ_SEC, DST_SEC, "ntp.ntsc.ac.cn", "ntp1.aliyun.com"); //ntp获取时间，你也可用其他"pool.ntp.org","0.cn.pool.ntp.org","1.cn.pool.ntp.org","ntp1.aliyun.com"
  delay(200);

}

void loop() {
  if (first) {  //首次加载
    updateDatas(&display);
    first = false;
  }
  if (millis() - timeSinceLastWUpdate > (1000L * UPDATE_INTERVAL_SECS)) { //屏幕刷新
    setReadyForWeatherUpdate();
    timeSinceLastWUpdate = millis();
  }
  if (millis() - timeSinceLastCurrUpdate > (1000L * UPDATE_CURR_INTERVAL_SECS)) { //粉丝数更新
    HeFengClient.fans(&currentWeather, BILIBILIID);
    fans = String(currentWeather.follower);
    timeSinceLastCurrUpdate = millis();
  }

  if (readyForWeatherUpdate && ui.getUiState()->frameState == FIXED) { //天气更新
    updateData(&display);
  }

  int remainingTimeBudget = ui.update(); //剩余时间预算

  if (remainingTimeBudget > 0) {
    //你可以在这里工作如果你低于你的时间预算。
    delay(remainingTimeBudget);
  }
  
}

void wificonnect() {  //WIFI密码连接，Web配网请注释
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(80);
    display.clear();
    display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_5);
    display.display();
    delay(80);
    display.clear();
    display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_6);
    display.display();
    delay(80);
    display.clear();
    display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_7);
    display.display();
    delay(80);
    display.clear();
    display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_8);
    display.display();
    delay(80);
    display.clear();
    display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_1);
    display.display();
    delay(80);
    display.clear();
    display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_2);
    display.display();
    delay(80);
    display.clear();
    display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_3);
    display.display();
    delay(80);
    display.clear();
    display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_4);
    display.display();
  }
  Serial.println("");
  delay(500);
}

//void webconnect() {  ////Web配网，密码直连将其注释
//  display.clear();
//  display.drawXbm(0, 0, 128, 64, bilibili); //显示哔哩哔哩
//  display.display();
//
//  WiFiManager wifiManager;  //实例化WiFiManager
//  wifiManager.setDebugOutput(false); //关闭Debug
//  //wifiManager.setConnectTimeout(10); //设置超时
//  wifiManager.setHeadImgBase64(FPSTR(Icon)); //设置图标
//  wifiManager.setPageTitle("欢迎来到小凯的WiFi配置页");  //设置页标题
//
//  if (!wifiManager.autoConnect("XiaoKai-IOT-Display")) {  //AP模式
//    Serial.println("连接失败并超时");
//    //重新设置并再试一次，或者让它进入深度睡眠状态
//    ESP.restart();
//    delay(1000);
//  }
//  Serial.println("connected...^_^");
//  yield();
//}

void drawProgress(OLEDDisplay *display, int percentage, String label) {    //绘制进度
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64, 10, label);
  display->drawProgressBar(2, 28, 124, 10, percentage);
  display->display();
}

void updateData(OLEDDisplay *display) {  //天气更新
  HeFengClient.doUpdateCurr(&currentWeather, HEFENG_KEY, HEFENG_LOCATION);
  HeFengClient.doUpdateFore(foreWeather, HEFENG_KEY, HEFENG_LOCATION);
  readyForWeatherUpdate = false;
}

void updateDatas(OLEDDisplay *display) {  //首次天气更新
  drawProgress(display, 0, "Updating fansnumb...");
  HeFengClient.fans(&currentWeather, BILIBILIID);
  fans = String(currentWeather.follower);
  
  drawProgress(display, 33, "Updating weather...");
  HeFengClient.doUpdateCurr(&currentWeather, HEFENG_KEY, HEFENG_LOCATION);
  
  drawProgress(display, 66, "Updating forecasts...");
  HeFengClient.doUpdateFore(foreWeather, HEFENG_KEY, HEFENG_LOCATION);
  
  readyForWeatherUpdate = false;
  drawProgress(display, 100, "Done...");
  delay(200);
  
}

void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {  //显示时间
  now = time(nullptr);
  struct tm* timeInfo;
  timeInfo = localtime(&now);
  char buff[16];

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  String date = WDAY_NAMES[timeInfo->tm_wday];

  sprintf_P(buff, PSTR("%04d-%02d-%02d  %s"), timeInfo->tm_year + 1900, timeInfo->tm_mon + 1, timeInfo->tm_mday, WDAY_NAMES[timeInfo->tm_wday].c_str());
  display->drawString(64 + x, 5 + y, String(buff));
  display->setFont(ArialMT_Plain_24);

  sprintf_P(buff, PSTR("%02d:%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
  display->drawString(64 + x, 22 + y, String(buff));
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {  //显示天气
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 38 + y, currentWeather.cond_txt + "    |   Wind: " + currentWeather.wind_sc + "  ");

  display->setFont(ArialMT_Plain_24);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  String temp = currentWeather.tmp + "°C" ;
  display->drawString(60 + x, 3 + y, temp);
  display->setFont(ArialMT_Plain_10);
  display->drawString(62 + x, 26 + y, currentWeather.fl + "°C | " + currentWeather.hum + "%");
  display->setFont(Meteocons_Plain_36);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(32 + x, 0 + y, currentWeather.iconMeteoCon);
}

void drawForecast(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {  //天气预报
  drawForecastDetails(display, x, y, 0);
  drawForecastDetails(display, x + 44, y, 1);
  drawForecastDetails(display, x + 88, y, 2);
}

void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex) {  //天气预报

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y, foreWeather[dayIndex].datestr);
  display->setFont(Meteocons_Plain_21);
  display->drawString(x + 20, y + 12, foreWeather[dayIndex].iconMeteoCon);

  String temp = foreWeather[dayIndex].tmp_max + " | " + foreWeather[dayIndex].tmp_min;
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y + 34, temp);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {   //绘图页眉覆盖
  now = time(nullptr);
  struct tm* timeInfo;
  timeInfo = localtime(&now);
  char buff[14];
  sprintf_P(buff, PSTR("%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min);

  display->setColor(WHITE);
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(6, 54, String(buff));
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  String temp = fans;
  display->drawString(122, 54, temp);
  display->drawHorizontalLine(0, 52, 128);
}

void setReadyForWeatherUpdate() {  //为天气更新做好准备
  Serial.println("Setting readyForUpdate to true");
  readyForWeatherUpdate = true;
}
