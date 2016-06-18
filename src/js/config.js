'use strict';
/* eslint-disable quotes */

module.exports = [
  {
    "type": "heading",
    "id": "main-heading",
    "defaultValue": "Dual Time Zone Configuration",
    "size": 1
  },
  
  {
    "type": "text",
    "defaultValue": "Use this configuration page to choose the settings you would like applied to Dual Time Zone!" 
  },
  
      {
      "type": "section",
      "capabilities": ["COLOR"],
      "items": [
      {
        "type": "heading",
        "defaultValue": "COLOR SELECTION"
      },
        
     {
        "type": "color",
        "messageKey": "LOCAL_BG_COLOR_KEY",
        "defaultValue": "0000FF",
        "label": "Local Time Background Color",
        "sunlight": false
      },
      {
        "type": "color",
        "messageKey": "TZ2_BG_COLOR_KEY",
        "defaultValue": "005500",
        "label": "Time Zone 2 Background Color",
        "sunlight": false
      },
      {
        "type": "color",
        "messageKey": "LOCAL_TEXT_COLOR_KEY",
        "defaultValue": "FFFFFF",
        "label": "Local Time Text Color",
        "sunlight": false
      },
      {
        "type": "color",
        "messageKey": "TZ2_TEXT_COLOR_KEY",
        "defaultValue": "FFFFFF",
        "label": "Time Zone 2 Text Color",
        "sunlight": false
      } 
       ]
      }, 
       
         //****************
      {
        "type": "section",
        "items": [
      {
        "type": "heading",
         "defaultValue": "DATE FORMAT:"
      },
      {
        "type": "radiogroup",
        "messageKey": "DATE_FORMAT_KEY",
        "label": "",
        "defaultValue": "0",
        "options": [
          { "label": "Jun 5, 2016", "value": "0" },
          { "label": "5 Jun 2016",  "value": "1" },
        ]
      }  
        ]
      },
          
      {
        "type": "section",
        "items": [
      {
        "type": "heading",
         "defaultValue": "FAHRENHEIT/CELSIUS:"
      },
      {
        "type": "radiogroup",
        "messageKey": "TEMP_FORMAT_KEY",
        "label": "",
        "defaultValue": "0",
        "options": [
          { "label": "Fahrenheit", "value": "0" },
          { "label": "Celsius",    "value": "1" },
       ]
      }  
      ]
      } , 
       //**********************
          
           {
        "type": "section",
        "items": [
      {
        "type": "heading",
         "defaultValue": "BLUETOOTH:"
      },
      {
        "type": "toggle",
        "messageKey": "BT_LOSS_KEY",
        "label": "Vibrate on Loss of Bluetooth",
        "defaultValue": false
      },
      ]
      },  
        //******************   
      {
        "type": "section",
        "items": [
      {
        "type": "heading",
         "defaultValue": "LOW BATTERY"
      },
      {
        "type": "toggle",
        "messageKey": "LOW_BATTERY_KEY",
        "label": "Vibrate on Low Battery",
        "defaultValue": false
      },
      ]
      },  
       //****************************
      {
        "type": "section",
        "items": [
      {
        "type": "heading",
         "defaultValue": ""
      },
      {
        "type": "input",
        "messageKey": "LOCATION_NAME_KEY",
        "defaultValue": "",
        "label": "LOCATION NAME",
        "description": "",          
        "attributes": {
        "placeholder": "Enter Location Here"
        }
      }     
  ]
},
       //*****************************
      {
        "type": "section",
        "items": [
          {
          "type": "text",
          "defaultValue": "OFFSET FROM LOCAL TIME",
          "capabilities": ["PLATFORM_APLITE"]
          },
          {
          "type": "text",
          "defaultValue": "UTC OFFSET",
          "capabilities": ["NOT_PLATFORM_APLITE"]
          },
      { 
        "type": "select",
         "messageKey": "UTC_OFFSET_KEY",
         "defaultValue": "+00:00",
         "label": "",
         "options": [
          {"label": "+11:00", "value": "+11:00" },
          {"label": "+10:00", "value": "+10:00" },
          {"label": "+09:30", "value": "+09:30" },
          {"label": "+09:00", "value": "+09:00" },
          {"label": "+08:45", "value": "+08:45" },
          {"label": "+08:30", "value": "+08:30" },
          {"label": "+08:00", "value": "+08:00" },
          {"label": "+07:00", "value": "+07:00" },
          {"label": "+06:30", "value": "+06:30" },
          {"label": "+06:00", "value": "+06:00" }, 
          {"label": "+05:30", "value": "+05:30" },
          {"label": "+05:00", "value": "+05:00" },
          {"label": "+04:30", "value": "+04:30" },
          {"label": "+04:00", "value": "+04:00" },
          {"label": "+03:30", "value": "+03:30" },
          {"label": "+03:00", "value": "+03:00" },
          {"label": "+02:00", "value": "+02:00" },
          {"label": "+01:00", "value": "+01:00" },
          {"label": "+00:00", "value": "+00:00" },
          {"label": "-01:00", "value": "-01:00" },
          {"label": "-02:00", "value": "-02:00" },
          {"label": "-03:00", "value": "-03:00" },
          {"label": "-03:30", "value": "-03:30" },
          {"label": "-04:00", "value": "-04:00" },
          {"label": "-04:30", "value": "-04:30" },
          {"label": "-05:00", "value": "-05:00" },
          {"label": "-06:00", "value": "-06:00" },
          {"label": "-07:00", "value": "-07:00" },
          {"label": "-08:00", "value": "-08:00" },
          {"label": "-09:00", "value": "-09:00" },
          {"label": "-09:30", "value": "-09:30" },
          {"label": "-10:00", "value": "-10:00" },
          {"label": "-11:00", "value": "-11:00" },
 {"label": "-12:00", "value": "-12:00" },
      ]
        }
          ]
      },
    {
    "type": "submit",
    "defaultValue": "Save"
    },
    {
    "type": "text",
    "defaultValue": "Dual Time Zone By WA1OUI V3.0",
          },
      //**************    
      {  
        "type": "section",
        "items": [
         {
           "type": "heading",
           "defaultValue": "TIME ZONE MAP"
         },  
          {
          "type": "text",
          "defaultValue": "Scroll to right to see zone offsets.",
          },
         {
         "type": "text",
         "defaultValue": "<img src='data:image/png;base64,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' /> ",
         }
      ]
  }
];   