#include "pebble.h"

#define LOCAL_BG_COLOR_KEY      0
#define TZ2_BG_COLOR_KEY        1
#define LOCAL_TEXT_COLOR_KEY    2
#define TZ2_TEXT_COLOR_KEY      3
#define DATE_FORMAT_KEY         4
#define BT_LOSS_KEY             5
#define LOW_BATTERY_KEY         6
#define UTC_OFFSET_KEY          7
#define LOCATION_NAME_KEY       8
#define WEATHER_TEMPERATURE_KEY 9
#define WEATHER_CITY_KEY        10
#define TEMP_FORMAT_KEY         11

Window *window;

TextLayer *text_local_layer;
TextLayer *text_TZ2_layer;
TextLayer *text_date_layer;
TextLayer *text_date2_layer;
TextLayer *text_time_layer;
TextLayer *text_time2_layer;
TextLayer *text_location_layer;
TextLayer *text_location2_layer;
TextLayer *text_degrees_layer;

Layer     *BatteryLineLayer;
Layer     *BTLayer;

GFont     fontMonaco13;
GFont     fontRobotoBoldSubset30;

GPoint     Linepoint;

static int BTConnected = 1;
static int BTVibesDone = 0;

static int  batterychargepct;
static int  BatteryVibesDone = 0;
static int  batterycharging = 0;
static int  FirstTime = 0;
static int  intUTCAdjust = 0;
static int  intUTCoffsethrs;
static int  intUTCoffsetmin;
static int  int_degrees  = 0;
static int  wxcall     = 0;

static char UTCOffsetConfig[]   = "+00:00";
static char strSign[]           = "+";
static char time_text[]         = "00:00a ";
static char time2_text[]        = "00:00a ";
static char gmttime_text[]      = "00:00a ";
static char date_text[]         = "  Dec 26, 2015";
static char date2_text[]        = "  Dec 26, 2015";
static char seconds_text[]      = "00";
static char date_format[]       = "  %a %m/%d/%y";
static char text_location[18]   = "N/A";
static char text_location2[18]  = "N/A";
static char text_degrees[]      = "N/A";

static int  PersistLocalBG      = 0;
static int  PersistTZ2BG        = 0;
static int  PersistLocalText    = 0;
static int  PersistTZ2Text      = 0;
static char PersistDateFormat[]  =  "0";   // 0 = US, 1 = Intl
static char PersistTempFormat[]  =  "0";   // 0 = F, 1 = C
static int  PersistBTLoss       = 0;
static int  PersistLow_Batt     = 0;
static char PersistUTCOffset[]  = "+00:00";
static char PersistLocationName[19];

static bool IsTimeZoneSet; //1 = Set, 0 ->gmtime==localtime

GColor TextColorHold1;
GColor TextColorHold2;
GColor BGColorHold2;
GColor ColorHold;

time_t local, utc;
   
void handle_battery(BatteryChargeState charge_state) {
  batterychargepct = charge_state.charge_percent;

  if (charge_state.is_charging) {
    batterycharging = 1;
  } else {
    batterycharging = 0;
  }

  // Reset if Battery > 20% ********************************
  if (batterychargepct > 20) {
     if (BatteryVibesDone == 1) {     //OK Reset to normal
         BatteryVibesDone = 0;
     } 
  }

  if ((batterychargepct < 30) && (PersistLow_Batt == 1) &&  (BatteryVibesDone == 0)) {   
         APP_LOG(APP_LOG_LEVEL_INFO, ". . . . Battery Vibes!");
         BatteryVibesDone = 1;
         vibes_long_pulse();
      }
  
    layer_mark_dirty(BatteryLineLayer);
}

void battery_line_layer_update_callback(Layer *BatteryLineLayer, GContext* batctx) { 
     graphics_context_set_fill_color(batctx, GColorWhite);
  
     graphics_fill_rect(batctx, layer_get_bounds(BatteryLineLayer), 3, GCornersAll);

     if (batterycharging == 1) {
          #ifdef PBL_PLATFORM_APLITE
             graphics_context_set_fill_color(batctx, GColorBlack);
          #else
             graphics_context_set_fill_color(batctx, GColorBlue);
          #endif
       
          graphics_fill_rect(batctx, GRect(2, 1, 100, 4), 3, GCornersAll);
     } else if (batterychargepct > 20) {
          #ifdef PBL_PLATFORM_APLITE
             graphics_context_set_fill_color(batctx, GColorBlack);
          #else
             graphics_context_set_fill_color(batctx, GColorGreen);
          #endif 
       
          graphics_fill_rect(batctx, GRect(2, 1, batterychargepct, 4), 3, GCornersAll);
     } else {   // Battery 20% or less 
          #ifdef PBL_PLATFORM_APLITE
             graphics_context_set_fill_color(batctx, GColorDarkGray);
          #else
             graphics_context_set_fill_color(batctx, GColorRed);
          #endif
       
          graphics_fill_rect(batctx, GRect(2, 1, batterychargepct, 4),3, GCornersAll);
     }
  
  //Battery % Markers
      #ifdef PBL_PLATFORM_APLITE
         if(batterycharging == 1) {
            graphics_context_set_fill_color(batctx, GColorBlack);
         } else {
            graphics_context_set_fill_color(batctx, GColorWhite);
         }
      #else
         graphics_context_set_fill_color(batctx, GColorBlack);
      #endif
  
      graphics_fill_rect(batctx, GRect(89, 1, 3, 4), 3, GCornerNone);
      graphics_fill_rect(batctx, GRect(79, 1, 3, 4), 3, GCornerNone);
      graphics_fill_rect(batctx, GRect(69, 1, 3, 4), 3, GCornerNone);
      graphics_fill_rect(batctx, GRect(59, 1, 3, 4), 3, GCornerNone);
      graphics_fill_rect(batctx, GRect(49, 1, 3, 4), 3, GCornerNone);
      graphics_fill_rect(batctx, GRect(39, 1, 3, 4), 3, GCornerNone);
      graphics_fill_rect(batctx, GRect(29, 1, 3, 4), 3, GCornerNone);
      graphics_fill_rect(batctx, GRect(19, 1, 3, 4), 3, GCornerNone);
      graphics_fill_rect(batctx, GRect(9,  1, 3, 4), 3, GCornerNone);
}

void handle_bluetooth(bool connected) {
      if (connected) {
         BTConnected = 1;     // Connected
         BTVibesDone = 0;
    } else {
         BTConnected = 0;      // Not Connected
    }
  
    layer_mark_dirty(BTLayer);
}

//BT Logo Callback;
void BTLine_update_callback(Layer *BTLayer, GContext* BT1ctx) {

       GPoint BTLinePointStart;
       GPoint BTLinePointEnd;
      
      if ((BTConnected == 0) && (PersistBTLoss == 1) && (BTVibesDone == 0)) {
              BTVibesDone = 1;
              vibes_long_pulse();
      }
  
      if(BTConnected == 0) {   
          graphics_context_set_stroke_color(BT1ctx, GColorRed);
          graphics_context_set_fill_color(BT1ctx, GColorWhite);
          graphics_fill_rect(BT1ctx, layer_get_bounds(BTLayer), 0, GCornerNone);
      
        // "X"" Line 1
          BTLinePointStart.x = 1;
          BTLinePointStart.y = 1;
  
          BTLinePointEnd.x = 20;
          BTLinePointEnd.y = 20;
          graphics_draw_line(BT1ctx, BTLinePointStart, BTLinePointEnd);   
          
         // "X"" Line 2
          BTLinePointStart.x = 1;
          BTLinePointStart.y =20;
  
          BTLinePointEnd.x = 20;
          BTLinePointEnd.y = 1;
          graphics_draw_line(BT1ctx, BTLinePointStart, BTLinePointEnd);
         
      } else {   
       BTVibesDone = 0; 
      
       graphics_context_set_stroke_color(BT1ctx, TextColorHold2);
       graphics_context_set_fill_color(BT1ctx, BGColorHold2);
  

       //Line 1
       BTLinePointStart.x = 10;
       BTLinePointStart.y = 1;
  
       BTLinePointEnd.x = 10;
       BTLinePointEnd.y = 20;
       graphics_draw_line(BT1ctx, BTLinePointStart, BTLinePointEnd);
         
       //Line 1a
       BTLinePointStart.x = 11;
       BTLinePointStart.y = 1;
  
       BTLinePointEnd.x = 11;
       BTLinePointEnd.y = 20;
       graphics_draw_line(BT1ctx, BTLinePointStart, BTLinePointEnd);
   
       //Line 2
       BTLinePointStart.x = 10;
       BTLinePointStart.y = 1;
  
       BTLinePointEnd.x = 17;
       BTLinePointEnd.y = 6;
       graphics_draw_line(BT1ctx, BTLinePointStart, BTLinePointEnd);
      
       //Line 2a
       BTLinePointStart.x = 11;
       BTLinePointStart.y = 1;
  
       BTLinePointEnd.x = 17;
       BTLinePointEnd.y = 5;
       graphics_draw_line(BT1ctx, BTLinePointStart, BTLinePointEnd);
 
       //Line 3
       BTLinePointStart.x = 4;
       BTLinePointStart.y = 5;
  
       BTLinePointEnd.x = 17;
       BTLinePointEnd.y = 15;
       graphics_draw_line(BT1ctx, BTLinePointStart, BTLinePointEnd);
  
       //Line 3a
       BTLinePointStart.x = 4;
       BTLinePointStart.y = 6;
  
       BTLinePointEnd.x = 17;
       BTLinePointEnd.y = 16;
       graphics_draw_line(BT1ctx, BTLinePointStart, BTLinePointEnd);

       //Line 4
       BTLinePointStart.x = 4;
       BTLinePointStart.y = 15;
  
       BTLinePointEnd.x = 17;
       BTLinePointEnd.y = 5;
       graphics_draw_line(BT1ctx, BTLinePointStart, BTLinePointEnd);
  
       //Line 4a
       BTLinePointStart.x = 4;
       BTLinePointStart.y = 16;
  
       BTLinePointEnd.x = 17;
       BTLinePointEnd.y = 6;
       graphics_draw_line(BT1ctx, BTLinePointStart, BTLinePointEnd);
  
       //Line 5
       BTLinePointStart.x = 10;
       BTLinePointStart.y = 20;
  
       BTLinePointEnd.x = 17;
       BTLinePointEnd.y = 15;
       graphics_draw_line(BT1ctx, BTLinePointStart, BTLinePointEnd);
  
       //Line 5a
       BTLinePointStart.x = 11;
       BTLinePointStart.y = 20;
  
       BTLinePointEnd.x = 17;
       BTLinePointEnd.y = 16;
       graphics_draw_line(BT1ctx, BTLinePointStart, BTLinePointEnd);   
      }            
}    
void ProcessTimeZone() {
  char strHours[]="12";
  char strMin[] = "30";

  memmove(strSign,  &PersistUTCOffset[0], 1);
  memmove(strHours, &PersistUTCOffset[1], 2);
  memmove(strMin,   &PersistUTCOffset[4], 2);
  
  intUTCoffsethrs = atoi(strHours);
  intUTCoffsetmin = atoi(strMin);

  if(strcmp(strSign, "+") == 0) {
      intUTCAdjust = (intUTCoffsethrs * 3600) + (intUTCoffsetmin * 60);
  } else {
      intUTCAdjust = -(intUTCoffsethrs * 3600) - (intUTCoffsetmin * 60);
  } 
    
}

void ProcessNoBTPersist() {
     APP_LOG(APP_LOG_LEVEL_ERROR, "In ProcessNoBTPersist * * *");
     if(persist_exists(LOCAL_BG_COLOR_KEY)) {
        PersistLocalBG = persist_read_int(LOCAL_BG_COLOR_KEY);
   
        APP_LOG(APP_LOG_LEVEL_INFO, "In ProcessNoBT, PersistLocalBG = %d", PersistLocalBG);
       
        ColorHold = GColorFromHEX(PersistLocalBG);
        
        text_layer_set_background_color(text_local_layer,    ColorHold);
        text_layer_set_background_color(text_location_layer, ColorHold);
        text_layer_set_background_color(text_degrees_layer,  ColorHold);
        text_layer_set_background_color(text_date_layer,     ColorHold);
        text_layer_set_background_color(text_time_layer,     ColorHold); 
     }
  
    if(persist_exists(TZ2_BG_COLOR_KEY)) {
       PersistTZ2BG = persist_read_int(TZ2_BG_COLOR_KEY);
     
        APP_LOG(APP_LOG_LEVEL_INFO, "                 PersistTZ2lBG = %d", PersistTZ2BG);
       
        ColorHold = GColorFromHEX(PersistTZ2BG);

       text_layer_set_background_color(text_TZ2_layer,       ColorHold);
       text_layer_set_background_color(text_location2_layer, ColorHold);
       text_layer_set_background_color(text_date2_layer,     ColorHold);
       text_layer_set_background_color(text_time2_layer,     ColorHold);
          
    }
  
    if(persist_exists(LOCAL_TEXT_COLOR_KEY)) {
       PersistLocalText = persist_read_int(LOCAL_TEXT_COLOR_KEY);
 
       APP_LOG(APP_LOG_LEVEL_INFO, "                 PersistLocalText = %d", PersistLocalText);
       
       ColorHold = GColorFromHEX(PersistLocalText);
  
       text_layer_set_text_color(text_local_layer,    ColorHold);
       text_layer_set_text_color(text_location_layer, ColorHold);
       text_layer_set_text_color(text_degrees_layer,  ColorHold);
       text_layer_set_text_color(text_date_layer,     ColorHold);
       text_layer_set_text_color(text_time_layer,     ColorHold);
    }
  
    if(persist_exists(TZ2_TEXT_COLOR_KEY)) {
       PersistTZ2Text = persist_read_int(TZ2_TEXT_COLOR_KEY);
    
       APP_LOG(APP_LOG_LEVEL_INFO, "                 PersistTZ2Text = %d", PersistTZ2Text);
       
       ColorHold = GColorFromHEX(PersistTZ2Text);
  
       TextColorHold2 = ColorHold; //Save for BT callback
       text_layer_set_text_color(text_TZ2_layer,       ColorHold);
       text_layer_set_text_color(text_location2_layer, ColorHold);
       text_layer_set_text_color(text_date2_layer,     ColorHold);
       text_layer_set_text_color(text_time2_layer,     ColorHold);
    }      
    //***********
  
    if(persist_exists(DATE_FORMAT_KEY)) {
       persist_read_string(DATE_FORMAT_KEY, PersistDateFormat, sizeof(PersistDateFormat));
      
       #ifdef PBL_PLATFORM_CHALK
             if (strcmp(PersistDateFormat, "1") == 0 ) {    // US
               strcpy(date_format,   " %a  %D");
             } else {
               strcpy(date_format,   " %a  %d/%m/%y");   //Intl
             }
         #else
             if (strcmp(PersistDateFormat, "1") == 0 ) { // US
                 strcpy(date_format, "%a   %D");
             } else {
                 strcpy(date_format, "%a   %d/%m/%y");   //Intl
             }     
         #endif
  
        text_layer_set_text(text_date_layer, date_text);
    }
    
    if(persist_exists(LOW_BATTERY_KEY)) {
                 PersistLow_Batt = persist_read_int(LOW_BATTERY_KEY);
    }
  
    if(persist_exists(UTC_OFFSET_KEY)) {
       persist_read_string(UTC_OFFSET_KEY, PersistUTCOffset, sizeof(PersistUTCOffset));
    }
           
    if(persist_exists(LOCATION_NAME_KEY)) {
          persist_read_string(LOCATION_NAME_KEY, PersistLocationName, sizeof(PersistLocationName));
          text_layer_set_text(text_location2_layer, PersistLocationName);     
       }
}     
//************************************************************************************************************
void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  char time_format[] = "%I:%M%p   ";
  char time_hold[]   = "09:02A";
  
  time_t seconds_since_epoch;
  seconds_since_epoch = time(NULL);

  strftime(seconds_text, sizeof(seconds_text), "%S", tick_time);

  //******************************************************************
  if((strcmp(seconds_text,"00") == 0) || (FirstTime == 0)) {
     FirstTime = 1;
      
    if(BTConnected == 0) {
        ProcessNoBTPersist();
    }  
    
     if (clock_is_24h_style()) {
        strcpy(time_format,"%R%"); //24 hour HH:MM
     } else {
        strcpy(time_format,"%I:%M%P"); //12 hour HH:MM   %p = AM/PM
     }
    
    //Process Local Time
     strftime(time_text, sizeof(time_text), time_format, tick_time);
    
     if(!clock_is_24h_style()) {
        strncpy(time_hold, time_text, 6);
        strcpy(time_text, time_hold);
     } 
    
     text_layer_set_text(text_time_layer,  time_text);
     //APP_LOG(APP_LOG_LEVEL_INFO, "%d seconds adjusted in Handle_tick", intUTCAdjust);

     //APP_LOG(APP_LOG_LEVEL_INFO, "%s = Local Time", time_text);
       
     //Process GMT for testing purposes
     struct tm* gmtinfo = gmtime(&seconds_since_epoch);
     strftime(gmttime_text, sizeof(time_text), time_format, gmtinfo);
    
     if(!clock_is_24h_style()) {
        strncpy(time_hold, gmttime_text, 6);
        strcpy(gmttime_text, time_hold);
     } 
     
     //APP_LOG(APP_LOG_LEVEL_INFO, "%s = GMT", gmttime_text);
    
    // Adjust for TZ2
     ProcessTimeZone(); //convert from character 12:32 to nunmber of seconds to adjust

     seconds_since_epoch = seconds_since_epoch + intUTCAdjust;
   
     //convert seconds since epoch into structure for gmt time
     struct tm* tz2info = gmtime(&seconds_since_epoch);
     strftime(time2_text, sizeof(time_text), time_format, tz2info);

    if(!clock_is_24h_style()) {
        strncpy(time_hold, time2_text, 6);
        strcpy(time2_text, time_hold);
     }   
     
     text_layer_set_text(text_time2_layer, time2_text);  

     //APP_LOG(APP_LOG_LEVEL_INFO, "%s = TZ2", time2_text);
     //APP_LOG(APP_LOG_LEVEL_ERROR, "==============================");
     
     strftime(date_text,  sizeof(date_text), date_format, tick_time);
     strftime(date2_text, sizeof(date2_text), date_format, gmtinfo);   
    
     text_layer_set_text(text_date_layer,  date_text);
     text_layer_set_text(text_date2_layer, date2_text);
   
    
  }  // End of First Time or 00 seconds
  
    
   if(((tick_time->tm_min % 15 == 0)  && (tick_time->tm_sec == 0)) || (wxcall  == 1)){

      // =================== Get weather:  
      // Only update wx/location every 15 minutes or when called
    
      APP_LOG(APP_LOG_LEVEL_ERROR, "In wx Processing %d:%d / wxcall = %d", tick_time->tm_min, tick_time->tm_sec, wxcall);
        
      // Begin dictionary
      DictionaryIterator *iter;
      app_message_outbox_begin(&iter);

      // Add a key-value pair
      dict_write_uint8(iter, 0, 0);

      // Send the message!
      APP_LOG(APP_LOG_LEVEL_INFO, "before outbox send in wx processing");
      app_message_outbox_send(); 
    }
  FirstTime = 1;
}
  
//<============================================================================================
//Receive Input from Config html page and location/temp:
void inbox_received_callback(DictionaryIterator *iterator, void *context) {
   char UTCTempHold[] = ":"; 
  //if(FirstTime == 0) { 
    //  FirstTime = 1;  
  //}  
   FirstTime = 0; 
  //****************
    APP_LOG(APP_LOG_LEVEL_WARNING, "In Inbox received callback*****************************************\n");
  
       #ifdef PBL_COLOR
          Tuple *Local_BG_Color = dict_find(iterator, LOCAL_BG_COLOR_KEY);     
            
         if (Local_BG_Color) { // Config Exists
             PersistLocalBG = Local_BG_Color->value->int32;
             ColorHold = GColorFromHEX(PersistLocalBG);
             persist_write_int(LOCAL_BG_COLOR_KEY,  PersistLocalBG );
        
             APP_LOG(APP_LOG_LEVEL_WARNING,    "    Added Config Local BG Color: %d", PersistLocalBG);   
         } else { //Check for Persist
               if(persist_exists(LOCAL_BG_COLOR_KEY)) {
                  PersistLocalBG = persist_read_int(LOCAL_BG_COLOR_KEY);
                  ColorHold = GColorFromHEX(PersistLocalBG);
                  APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Persistant Local BG Color = %d", PersistLocalBG);
               }  else {   // Set Default
                  PersistLocalBG = 255; 
                  ColorHold = GColorFromHEX(PersistLocalBG);

                  APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Default Local BG Color %d", PersistLocalBG);
               }
            }
          
         persist_write_int(LOCAL_BG_COLOR_KEY,   PersistLocalBG);
        
       #else
         ColorHold = GColorBlack;
       #endif
  
         text_layer_set_background_color(text_local_layer,    ColorHold);
         text_layer_set_background_color(text_location_layer, ColorHold);
         text_layer_set_background_color(text_degrees_layer,  ColorHold);
         text_layer_set_background_color(text_date_layer,     ColorHold);
         text_layer_set_background_color(text_time_layer,     ColorHold);

  //****************
        
     #ifdef PBL_COLOR
        Tuple *TZ2_BG_Color =  dict_find(iterator, TZ2_BG_COLOR_KEY);
 
         if (TZ2_BG_Color) { // Config Exists
             PersistTZ2BG = TZ2_BG_Color->value->int32;
             ColorHold = GColorFromHEX(PersistTZ2BG);
             persist_write_int(TZ2_BG_COLOR_KEY,  PersistTZ2BG);
        
             APP_LOG(APP_LOG_LEVEL_WARNING,    "    Added Config TZ2 BG Color: %d", PersistTZ2BG);   
         } else { //Check for Persist
               if(persist_exists(TZ2_BG_COLOR_KEY)) {
                  PersistTZ2BG = persist_read_int(TZ2_BG_COLOR_KEY);
                  ColorHold = GColorFromHEX(PersistTZ2BG);
                  APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Persistant TZ2 BG Color = %d", PersistTZ2BG);
               }  else {   // Set Default
                  PersistTZ2BG = 21760; 
                  ColorHold = GColorFromHEX(PersistTZ2BG);

                  APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Default TZ2 BG Color %d", PersistTZ2BG);
               }
            }
         
         persist_write_int(TZ2_BG_COLOR_KEY, PersistTZ2BG);
       #else
         ColorHold = GColorWhite;
       #endif
  
         text_layer_set_background_color(text_TZ2_layer,       ColorHold);
         text_layer_set_background_color(text_location2_layer, ColorHold);
         text_layer_set_background_color(text_date2_layer,     ColorHold);
         text_layer_set_background_color(text_time2_layer,     ColorHold);
  
  //****************

    #ifdef PBL_COLOR
        Tuple *Local_Text_Color =  dict_find(iterator, LOCAL_TEXT_COLOR_KEY);
 
         if (TZ2_BG_Color) { // Config Exists
             PersistLocalText = Local_Text_Color->value->int32;
             ColorHold = GColorFromHEX(PersistLocalText);
             persist_write_int(LOCAL_TEXT_COLOR_KEY, Local_Text_Color->value->int32);
        
             APP_LOG(APP_LOG_LEVEL_WARNING,    "    Added Config Local Text Color: %d", PersistLocalText);   
         } else { //Check for Persist
               if(persist_exists(LOCAL_TEXT_COLOR_KEY)) {
                  PersistLocalText = persist_read_int(LOCAL_TEXT_COLOR_KEY);
                  ColorHold = GColorFromHEX(PersistLocalText);
                  APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Persistant Local Text Color = %d", PersistLocalText);
               }  else {   // Set Default
                  PersistLocalText = 16777215; 
                    ColorHold = GColorFromHEX(PersistLocalText);

                  APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Default Local Text %d", PersistLocalText);
               }
            }
         
         persist_write_int(LOCAL_TEXT_COLOR_KEY, PersistLocalText);
       #else
         ColorHold = GColorWhite;
       #endif
  
         text_layer_set_text_color(text_local_layer,    ColorHold);
         text_layer_set_text_color(text_location_layer, ColorHold);
         text_layer_set_text_color(text_degrees_layer,  ColorHold);
         text_layer_set_text_color(text_date_layer,     ColorHold);
         text_layer_set_text_color(text_time_layer,     ColorHold);
  
    
      //****************
       #ifdef PBL_COLOR
        Tuple *TZ2_Text_Color =  dict_find(iterator, TZ2_TEXT_COLOR_KEY);
 
         if (TZ2_Text_Color) { // Config Exists
             PersistTZ2Text = TZ2_Text_Color->value->int32;
             ColorHold = GColorFromHEX(PersistTZ2Text);
             persist_write_int(TZ2_TEXT_COLOR_KEY,TZ2_Text_Color->value->int32);
        
             APP_LOG(APP_LOG_LEVEL_WARNING,    "    Added Config TZ2 Text Color: %d", PersistTZ2Text);   
         } else { //Check for Persist
               if(persist_exists(TZ2_TEXT_COLOR_KEY)) {
                  PersistLocalText = persist_read_int(TZ2_TEXT_COLOR_KEY);
                  ColorHold = GColorFromHEX(PersistTZ2Text);
                  APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Persistant TZ2 Text Color = %d", PersistTZ2Text);
               }  else {   // Set Default
                  PersistTZ2Text =16777215; 
                  ColorHold = GColorFromHEX(PersistTZ2Text);

                  APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Default TZ2 Text Color = %d", PersistTZ2Text);

               }
            }
         
         persist_write_int(TZ2_TEXT_COLOR_KEY, PersistTZ2Text);
       #else
         ColorHold = GColorBlack;
       #endif
  
   
         TextColorHold2 = ColorHold; //Save for BT callback
         text_layer_set_text_color(text_TZ2_layer,       ColorHold);
         text_layer_set_text_color(text_location2_layer, ColorHold);
         text_layer_set_text_color(text_date2_layer,     ColorHold);
         text_layer_set_text_color(text_time2_layer,     ColorHold);

  //****************
  
        Tuple *Date_Format = dict_find(iterator, DATE_FORMAT_KEY);
        
        if(Date_Format && ((strcmp(Date_Format->value->cstring, "0") == 0) || (strcmp(Date_Format->value->cstring, "1") == 0))) {
             strcpy(PersistDateFormat,Date_Format->value->cstring);

            APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Config Value Date Format = %s, 0 = US, 1 = Intl", PersistDateFormat);
         } else {
            if(persist_exists(DATE_FORMAT_KEY)) {
               persist_read_string(DATE_FORMAT_KEY, PersistDateFormat, sizeof(PersistDateFormat));

               APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Persistant Date Format = %s, (0 = US, 1 = Intl)", PersistDateFormat);
            } else {
               strcpy(PersistDateFormat, "0"); // US Default
               APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Default Date Format (US) = %s", PersistDateFormat);
            }             
         }
          
         persist_write_string(DATE_FORMAT_KEY, PersistDateFormat);

         #ifdef PBL_PLATFORM_CHALK
             if (strcmp(PersistDateFormat, "0") == 0) {     // US
               strcpy(date_format,   " %a  %D");
             } else {
               strcpy(date_format,   " %a  %d/%m/%y");       //Intl
             }
         #else
             if (strcmp(PersistDateFormat, "0") == 0) {       // US
                 strcpy(date_format, "%a   %D");
             } else {
                 strcpy(date_format, "%a   %d/%m/%y");        //Intl
             }     
         #endif
  
        text_layer_set_text(text_date_layer, date_text);

  //****************
                 
        Tuple *BT_LossVib = dict_find(iterator, BT_LOSS_KEY); 

       if((BT_LossVib) && ((BT_LossVib->value->uint8 == 0) || (BT_LossVib->value->uint8 == 1))) {        
           PersistBTLoss = BT_LossVib->value->uint8; //Vibe on loss
           APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Config BT Loss Key = %d (0=No Vib, 1=Vib)", PersistBTLoss);
       } else {
         if(persist_exists(BT_LOSS_KEY)) {
               PersistBTLoss = persist_read_int(BT_LOSS_KEY);
               APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Persistant BT Loss = %d, (0 = No Vib, 1 = Vib)", PersistBTLoss);
            } else {
               PersistBTLoss = 0; // No Vib on BT Loss
               APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Default BT Loss Key = %d (No Vibrate on Loss)", PersistBTLoss);
            }
       }
        persist_write_int(BT_LOSS_KEY, PersistBTLoss);

 //******************
                  
        Tuple *Low_Battery_Vib = dict_find(iterator, LOW_BATTERY_KEY); 

       if((Low_Battery_Vib) && ((Low_Battery_Vib->value->uint8 == 0) || (Low_Battery_Vib->value->uint8 == 1))) {        
           PersistLow_Batt = BT_LossVib->value->uint8; //Vibe on loss
           APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Config Low Batt Key = %d (0=No Vib, 1=Vib)", PersistLow_Batt);
       } else {
         if(persist_exists(LOW_BATTERY_KEY)) {
               PersistLow_Batt = persist_read_int(LOW_BATTERY_KEY);
               APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Persistant Low Batt Key = %d, (0 = No Vib, 1 = Vib)", PersistLow_Batt);
            } else {
               PersistLow_Batt= 0; // No Vib on BT Loss
               APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Default Low Batt Key = %d (No Vibrate on Loss)", PersistLow_Batt);
            }
       }
        persist_write_int(LOW_BATTERY_KEY, PersistLow_Batt);
  
  //******************
        Tuple *UTC_Offset = dict_find(iterator, UTC_OFFSET_KEY);

        if(UTC_Offset) {  // config value exists
           strcpy(UTCOffsetConfig, UTC_Offset->value->cstring);
           memmove(UTCTempHold, &UTCOffsetConfig[3], 1);
           APP_LOG(APP_LOG_LEVEL_WARNING,    "    UTC Config Value = %s", UTCOffsetConfig);
           APP_LOG(APP_LOG_LEVEL_WARNING,    "    UTC Test Char = %s", UTCTempHold);  
          
           if(strcmp(UTCTempHold, ":") == 0) {
              strcpy(PersistUTCOffset, UTCOffsetConfig); 
              APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Config UTC Offset = %s", PersistUTCOffset);
           }  
           } else { 
              if(persist_exists(UTC_OFFSET_KEY)) {
                 persist_read_string(UTC_OFFSET_KEY, PersistUTCOffset, sizeof(PersistUTCOffset));
                 APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Persistant UTC Offset = %s", PersistUTCOffset);
              } else {
               strcpy(PersistUTCOffset, "+00:00"); // Default to UTC
               APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Default Persistant UTC Offset = %s", PersistUTCOffset);
              }
           }  
        
           persist_write_string(UTC_OFFSET_KEY, PersistUTCOffset);

  //******************
        
         Tuple *Location_name = dict_find(iterator, LOCATION_NAME_KEY);
        
         if(Location_name) {
            strcpy(PersistLocationName, Location_name->value->cstring);
            APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Config TZ2 Location:  %s", PersistLocationName);
         } else {
            if(persist_exists(LOCATION_NAME_KEY)) {
               persist_read_string(LOCATION_NAME_KEY, PersistLocationName, sizeof(PersistLocationName));
               APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Persistant TZ2 Location = %s", PersistLocationName);
            } else {
               strcpy(PersistLocationName, "UTC"); // Default to UTC
               APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Default TZ2 Location = %s", PersistLocationName);
            }
         }  
             
         text_layer_set_text(text_location2_layer, PersistLocationName);
         
         persist_write_string(LOCATION_NAME_KEY,   PersistLocationName);
       
 //******************
  
         if(wxcall == 1) { // reset flag to process wx
            wxcall = 0;
        } else {
           wxcall = 1; 
         } 
  
        Tuple *Temp_Type = dict_find(iterator, TEMP_FORMAT_KEY);
        if(Temp_Type && ((strcmp(Temp_Type->value->cstring, "0") == 0) || (strcmp(Temp_Type->value->cstring, "1") == 0))) {
             strcpy(PersistTempFormat,Temp_Type->value->cstring);
             APP_LOG(APP_LOG_LEVEL_WARNING,     "    Added Config Temp Type = %s, (1 = F. 0 = C)\n", PersistTempFormat);
       } else {
         if(persist_exists(TEMP_FORMAT_KEY)) {
               persist_read_string(TEMP_FORMAT_KEY, PersistTempFormat, sizeof(PersistTempFormat));
               APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Persistant Temp Type = %s, (1 = F, 0 = C)\n", PersistTempFormat);
            } else {
               strcpy(PersistTempFormat,"0"); //Default F
               APP_LOG(APP_LOG_LEVEL_WARNING, "    Added Default Temp Type = %s, (Degrees F)\n", PersistTempFormat);
            }
       }
        persist_write_string(TEMP_FORMAT_KEY, PersistTempFormat);
  
  //******************
    
            strcpy(text_degrees, "===");  // Reset
  
            APP_LOG(APP_LOG_LEVEL_WARNING, "Processing Temp & Location...");
       
            Tuple *Temperature = dict_find(iterator, WEATHER_TEMPERATURE_KEY);
            
            if(Temperature) {
                int_degrees = Temperature->value->uint8;
            } else {
                strcpy(text_degrees, "N/A");
            }
                   
             if (strcmp((text_degrees), "N/A") != 0) {
                APP_LOG(APP_LOG_LEVEL_WARNING,   "    PersistTempFormat = %s, (0 = F, 1 = C)", PersistTempFormat);
               
                APP_LOG(APP_LOG_LEVEL_WARNING,   "    Input Degrees C = %d", int_degrees);
          
                if(strcmp(PersistTempFormat, "0") == 0)    { // Degrees F  
                   APP_LOG(APP_LOG_LEVEL_WARNING, "       Converting C to F");
                   int_degrees = ((int_degrees * 9) / 5) + 32;
  
                   // Assemble full string and display

                  snprintf(text_degrees, 5, "%dF ", int_degrees);  
                  APP_LOG(APP_LOG_LEVEL_WARNING, "    Output Degrees F = %s", text_degrees);

                } else {
                  snprintf(text_degrees, 5, "%dC ", int_degrees); //Temp Degrees C
                }  
           } 

           text_layer_set_text(text_degrees_layer, text_degrees); 
           APP_LOG(APP_LOG_LEVEL_WARNING, "    Temp Output: %s\n", text_degrees);        
       
          //******************
          APP_LOG(APP_LOG_LEVEL_WARNING, "Processing Wx Location Name...");
         
          Tuple *Wx_City = dict_find(iterator, WEATHER_CITY_KEY);
         
          strcpy(text_location,Wx_City->value->cstring) ;      
          text_layer_set_text(text_location_layer, text_location);   
          APP_LOG(APP_LOG_LEVEL_WARNING, "    Location = %s\n", text_location);      
}

void handle_appfocus(bool in_focus){
    if (in_focus) {
        FirstTime = 0;        
    }
}
void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox Message dropped!");
}

void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

void handle_deinit(void) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  app_focus_service_unsubscribe();
  app_message_deregister_callbacks();
  
  text_layer_destroy(text_time_layer);
  text_layer_destroy(text_time2_layer);
  text_layer_destroy(text_date_layer);
  text_layer_destroy(text_date2_layer);
  text_layer_destroy(text_location_layer);
  text_layer_destroy(text_location2_layer);
  text_layer_destroy(text_degrees_layer);
  text_layer_destroy(text_local_layer);
  text_layer_destroy(text_TZ2_layer);
   
  layer_destroy(BatteryLineLayer);   
  layer_destroy(BTLayer);

  fonts_unload_custom_font(fontMonaco13);
  fonts_unload_custom_font(fontRobotoBoldSubset30);

  //window_destroy(window);
}

void handle_init(void) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Enterng INIT************************************************************");
  APP_LOG(APP_LOG_LEVEL_ERROR, "*************************************************************************");

  FirstTime = 0;
  
  setlocale(LC_TIME, "");
  
 #ifdef PBL_BW
     GColor BGCOLOR1 = GColorBlack;
     GColor BGCOLOR2 = GColorWhite;
     BGColorHold2    = GColorWhite;

     GColor TEXTCOLOR1 = GColorWhite;
     GColor TEXTCOLOR2 = GColorBlack;
     TextColorHold1   = GColorWhite;
     TextColorHold2   = GColorBlack;
  #else
     GColor BGCOLOR1 = GColorBlue;
     GColor BGCOLOR2 = GColorDarkGreen;
     BGColorHold2 = GColorDarkGreen;

     GColor TEXTCOLOR1 = GColorWhite;
     GColor TEXTCOLOR2 = GColorWhite;
     TextColorHold1   = GColorWhite;
     TextColorHold2   = GColorWhite;
 #endif
  
  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, BGCOLOR1);

  fontMonaco13           = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MONACO_13));
  fontRobotoBoldSubset30 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_30));
  
  Layer *window_layer = window_get_root_layer(window);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(256, 256);

  //Local Time Layer
  #ifdef PBL_PLATFORM_CHALK
    text_local_layer = text_layer_create(GRect(1, 1, 180, 90)); 
  #else
    text_local_layer = text_layer_create(GRect(1, 1, 144, 84)); 
  #endif 
  
  text_layer_set_background_color(text_local_layer, BGCOLOR1);
  layer_add_child(window_layer, text_layer_get_layer(text_local_layer));
  
  //TZ2 Layer
  #ifdef PBL_PLATFORM_CHALK
    text_TZ2_layer = text_layer_create(GRect(1, 90, 180, 90)); 
  #else
    text_TZ2_layer = text_layer_create(GRect(1, 84, 144, 84));   
  #endif
 
  text_layer_set_background_color(text_TZ2_layer, BGCOLOR2);
  layer_add_child(window_layer, text_layer_get_layer(text_TZ2_layer));

  //Location 1 - Local
  #ifdef PBL_PLATFORM_CHALK
    text_location_layer = text_layer_create(GRect(1, 44, 180, 17)); 
  #else
    text_location_layer = text_layer_create(GRect(1, 1, 144, 17)); 
  #endif
  
  text_layer_set_text_alignment(text_location_layer, GTextAlignmentCenter);		
  text_layer_set_text(text_location_layer, text_location); 
  text_layer_set_font(text_location_layer, fontMonaco13);
  text_layer_set_background_color(text_location_layer, BGCOLOR1);
  text_layer_set_text_color(text_location_layer, TEXTCOLOR1);
  layer_add_child(window_layer, text_layer_get_layer(text_location_layer));
  
  // Date 1
  #ifdef PBL_PLATFORM_CHALK
    text_date_layer = text_layer_create(GRect(3, 57, 135, 22));
    text_layer_set_text_alignment(text_date_layer, GTextAlignmentLeft);;
  #else
    text_date_layer = text_layer_create(GRect(1, 18, 144, 22));
    text_layer_set_text_alignment(text_date_layer, GTextAlignmentCenter);; 
  #endif
  
  text_layer_set_text_color(text_date_layer, TEXTCOLOR1);
  text_layer_set_background_color(text_date_layer, BGCOLOR1);
  text_layer_set_font(text_date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  layer_add_child(window_layer, text_layer_get_layer(text_date_layer));
  
 
  // Time of Day 1
  #ifdef PBL_PLATFORM_CHALK
     text_time_layer = text_layer_create(GRect(1, 6, 180, 40)); 
  #else
     text_time_layer = text_layer_create(GRect(5, 40, 100, 40)); 
  #endif
  
  text_layer_set_font(text_time_layer, fontRobotoBoldSubset30); 
  text_layer_set_text_color(text_time_layer, TEXTCOLOR1);
  text_layer_set_background_color(text_time_layer, BGCOLOR1);
  text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));
  
   //Temperature
  #ifdef PBL_PLATFORM_CHALK
    text_degrees_layer = text_layer_create(GRect(141, 63, 39, 17)); 
    text_layer_set_text_alignment(text_degrees_layer, GTextAlignmentLeft);		
  #else
     text_degrees_layer = text_layer_create(GRect(100, 55, 40, 17));
     text_layer_set_text_alignment(text_degrees_layer, GTextAlignmentRight);		
  #endif
 
  text_layer_set_text(text_degrees_layer, text_degrees); 
  text_layer_set_font(text_degrees_layer, fontMonaco13);
  text_layer_set_background_color(text_degrees_layer, BGCOLOR1);
  text_layer_set_text_color(text_degrees_layer, TEXTCOLOR1);
  layer_add_child(window_layer, text_layer_get_layer(text_degrees_layer));
  
    // Battery Line
  #ifdef PBL_PLATFORM_CHALK
      GRect battery_line_frame = GRect(38, 87, 104, 6);
  #else
      GRect battery_line_frame = GRect(22, 81, 104, 6);
  #endif
  
  BatteryLineLayer = layer_create(battery_line_frame);
  layer_set_update_proc(BatteryLineLayer, battery_line_layer_update_callback);
  layer_add_child(window_layer, BatteryLineLayer);
  
  
  
  // Date 2
  #ifdef PBL_PLATFORM_CHALK
     text_date2_layer = text_layer_create(GRect(3, 94, 177, 29));
     text_layer_set_text_alignment(text_date2_layer, GTextAlignmentLeft);;
  #else
     text_date2_layer = text_layer_create(GRect(1, 106, 144, 22));
     text_layer_set_text_alignment(text_date2_layer, GTextAlignmentCenter);;
  #endif
  
  text_layer_set_text_color(text_date2_layer, TEXTCOLOR2);
  text_layer_set_background_color(text_date2_layer, BGCOLOR2);
  text_layer_set_font(text_date2_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  layer_add_child(window_layer, text_layer_get_layer(text_date2_layer));

  // Time 2
  #ifdef PBL_PLATFORM_CHALK
     text_time2_layer = text_layer_create(GRect(1, 135, 180, 40));
  #else
    text_time2_layer = text_layer_create(GRect(5, 127, 100, 40));
  #endif
  
  text_layer_set_font(text_time2_layer,fontRobotoBoldSubset30);
  text_layer_set_text_color(text_time2_layer, TEXTCOLOR2);
  text_layer_set_background_color(text_time2_layer, BGCOLOR2);
  text_layer_set_text_alignment(text_time2_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_time2_layer)); 

  //Location 2
  #ifdef PBL_PLATFORM_CHALK
      text_location2_layer = text_layer_create(GRect(1, 123, 180, 19)); 
  #else
      text_location2_layer = text_layer_create(GRect(1, 92, 144, 19));     
  #endif
  
  text_layer_set_text_alignment(text_location2_layer, GTextAlignmentCenter);		
  text_layer_set_text(text_location2_layer, text_location2); 
  text_layer_set_font(text_location2_layer, fontMonaco13);
  text_layer_set_background_color(text_location2_layer, BGCOLOR2);
  text_layer_set_text_color(text_location2_layer, TEXTCOLOR2);
  layer_add_child(window_layer, text_layer_get_layer(text_location2_layer));
  
  //Bluetooth Logo Setup area
  #ifdef PBL_PLATFORM_CHALK
     GRect BTArea = GRect(139, 96, 20, 20);
  #else
     GRect BTArea = GRect(110, 138, 20, 20);
  #endif
  
  BTLayer = layer_create(BTArea);

  layer_add_child(window_layer, BTLayer);
  layer_set_update_proc(BTLayer, BTLine_update_callback); 
    
  bluetooth_connection_service_subscribe(&handle_bluetooth);

  battery_state_service_subscribe(&handle_battery);
  bluetooth_connection_service_subscribe(&handle_bluetooth);
  app_focus_service_subscribe(&handle_appfocus);

  handle_battery(battery_state_service_peek());
  handle_bluetooth(bluetooth_connection_service_peek());
  
  ProcessNoBTPersist();
    
  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);

  IsTimeZoneSet = clock_is_timezone_set();
  
  APP_LOG(APP_LOG_LEVEL_WARNING, "Time Zone set = %d, 1 = Yes, 0 ->GMTime = localtime", IsTimeZoneSet);
  
  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_tick(current_time, SECOND_UNIT);


 
}
int main(void) {
   handle_init();

   app_event_loop();

   handle_deinit();
}
