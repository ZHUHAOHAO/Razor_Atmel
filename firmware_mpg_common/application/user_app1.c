/**********************************************************************************************************************
File: user_app.c                                                                

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app as a template:
 1. Copy both user_app.c and user_app.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "user_app" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "UserApp1" with "YourNewTaskName"
 6. Use ctrl-h to find and replace all instances of "USER_APP" with "YOUR_NEW_TASK_NAME"
 7. Add a call to YourNewTaskNameInitialize() in the init section of main
 8. Add a call to YourNewTaskNameRunActiveState() in the Super Loop section of main
 9. Update yournewtaskname.h per the instructions at the top of yournewtaskname.h
10. Delete this text (between the dashed lines) and update the Description below to describe your task
----------------------------------------------------------------------------------------------------------------------

Description:
This is a user_app.c file template 

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
void UserApp1Initialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void UserApp1RunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp1Flags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern u32 G_u32AntApiCurrentDataTimeStamp;                       /* From ant_api.c */
extern AntApplicationMessageType G_eAntApiCurrentMessageClass;    /* From ant_api.c */
extern u8 G_au8AntApiCurrentMessageBytes[ANT_APPLICATION_MESSAGE_BYTES];  /* From ant_api.c */
extern AntExtendedDataType G_sAntApiCurrentMessageExtData;        /* From ant_api.c */

extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */



/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp1_" and be declared as static.
***********************************************************************************************************************/
static u32 UserApp1_u32DataMsgCount = 0;             /* Counts the number of ANT_DATA packets received */
static u32 UserApp1_u32TickMsgCount = 0;             /* Counts the number of ANT_TICK packets received */

static fnCode_type UserApp1_StateMachine;            /* The state machine function pointer */
static u32 UserApp1_u32Timeout;                      /* Timeout counter used across states */


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: UserApp1Initialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - 
*/
void UserApp1Initialize(void)
{
  static u8 au8WelcomeWords[]="Hide and Go Seek!";
  static u8 au8WelcomeWords1[]="Press B0 to Start";
  
  static bool bStart = TRUE;
  if(bStart)
  {
    LCDCommand(LCD_CLEAR_CMD);
    LCDMessage(LINE1_START_ADDR, au8WelcomeWords);
    LCDMessage(LINE2_START_ADDR, au8WelcomeWords1);
    bStart = FALSE;
  }
  
  UserApp1_StateMachine = UserApp1SM_SeekerSet;
    
 
} /* end UserApp1Initialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function UserApp1RunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void UserApp1RunActiveState(void)
{
  UserApp1_StateMachine();

} /* end UserApp1RunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for the ANT channel assignment to finish */
static void UserApp1SM_WaitChannelAssign(void)
{
  /* Check if the channel assignment is complete */
  if(AntRadioStatusChannel(ANT_CHANNEL_USERAPP) == ANT_CONFIGURED)
  {
#ifdef EIE1
    LedOff(RED);
    //LedOn(YELLOW);
#endif /* EIE1 */
    
#ifdef MPG2
    LedOff(RED0);
    LedOn(GREEN0);
#endif /* MPG2 */

    UserApp1_StateMachine = UserApp1SM_Idle;
  }
  
  /* Monitor for timeout */
  if( IsTimeUp(&UserApp1_u32Timeout, 5000) )
  {
    DebugPrintf("\n\r***Channel assignment timeout***\n\n\r");
    UserApp1_StateMachine = UserApp1SM_Error;
  }
      
} /* end UserApp1SM_WaitChannelAssign() */


  /*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a message to be queued */
static void UserApp1SM_Idle(void)
{
  
  /* Look for BUTTON 0 to open channel */
  if(WasButtonPressed(BUTTON0))
  {
    /* Got the button, so complete one-time actions before next state */
    ButtonAcknowledge(BUTTON0);
    /* Queue open channel and change LED0 from yellow to blinking green to indicate channel is opening */
    AntOpenChannelNumber(ANT_CHANNEL_USERAPP);

    /* Set timer and advance states */
  
    UserApp1_u32Timeout = G_u32SystemTime1ms;
    UserApp1_StateMachine = UserApp1SM_WaitChannelOpen;
  
  }
} /* end UserApp1SM_Idle() */
     

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for channel to open */
static void UserApp1SM_WaitChannelOpen(void)
{
  /* Monitor the channel status to check if channel is opened */
  if(AntRadioStatusChannel(ANT_CHANNEL_USERAPP) == ANT_OPEN)
  {
    UserApp1_StateMachine = UserApp1SM_HiderSet;
    
  }
  
  /* Check for timeout */
  if( IsTimeUp(&UserApp1_u32Timeout, TIMEOUT_VALUE) )
  {
    AntCloseChannelNumber(ANT_CHANNEL_USERAPP);

    UserApp1_StateMachine = UserApp1SM_Idle;
  }
    
} /* end UserApp1SM_WaitChannelOpen() */


/*-------------------------------------------------------------------------------------------------------------------*/
/*Configure seeker*/
static void UserApp1SM_SeekerSet(void)
{
  AntAssignChannelInfoType sAntSetupData;
  sAntSetupData.AntChannel          = ANT_CHANNEL_USERAPP;
  sAntSetupData.AntChannelType      = ANT_CHANNEL_TYPE_USERAPP;
  sAntSetupData.AntChannelPeriodLo  = ANT_CHANNEL_PERIOD_LO_USERAPP;
  sAntSetupData.AntChannelPeriodHi  = ANT_CHANNEL_PERIOD_HI_USERAPP;
  
  sAntSetupData.AntDeviceIdLo       = ANT_DEVICEID_LO_USERAPP;
  sAntSetupData.AntDeviceIdHi       = ANT_DEVICEID_HI_USERAPP;
  sAntSetupData.AntDeviceType       = ANT_DEVICE_TYPE_USERAPP;
  sAntSetupData.AntTransmissionType = ANT_TRANSMISSION_TYPE_USERAPP;
  sAntSetupData.AntFrequency        = ANT_FREQUENCY_USERAPP;
  sAntSetupData.AntTxPower          = ANT_TX_POWER_USERAPP;
  
  sAntSetupData.AntNetwork = ANT_NETWORK_DEFAULT;
  
  
  for(u8 i = 0; i < ANT_NETWORK_NUMBER_BYTES; i++)
  {
    sAntSetupData.AntNetworkKey[i] = ANT_DEFAULT_NETWORK_KEY;
  }
    
  /* If good initialization, set state to Idle */
  if( AntAssignChannel(&sAntSetupData) )
  {
    /* Channel assignment is queued so start timer */
#ifdef EIE1
    UserApp1_u32Timeout = G_u32SystemTime1ms;
    //LedOn(RED);
#endif /* EIE1 */
    
#ifdef MPG2
    LedOn(RED0);
#endif /* MPG2 */
    
    UserApp1_StateMachine = UserApp1SM_WaitChannelAssign;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp1_StateMachine = UserApp1SM_Error;
  }
}

/*Configure hider*/
static void UserApp1SM_HiderSet(void)
{
  AntAssignChannelInfoType sAntSetupData1;
  sAntSetupData1.AntChannel          = ANT_CHANNEL_USERAPP1;
  sAntSetupData1.AntChannelType      = ANT_CHANNEL_TYPE_USERAPP1;
  sAntSetupData1.AntChannelPeriodLo  = ANT_CHANNEL_PERIOD_LO_USERAPP;
  sAntSetupData1.AntChannelPeriodHi  = ANT_CHANNEL_PERIOD_HI_USERAPP;
  
  sAntSetupData1.AntDeviceIdLo       = ANT_DEVICEID_LO_USERAPP;
  sAntSetupData1.AntDeviceIdHi       = ANT_DEVICEID_HI_USERAPP;
  sAntSetupData1.AntDeviceType       = ANT_DEVICE_TYPE_USERAPP;
  sAntSetupData1.AntTransmissionType = ANT_TRANSMISSION_TYPE_USERAPP;
  sAntSetupData1.AntFrequency        = ANT_FREQUENCY_USERAPP;
  sAntSetupData1.AntTxPower          = ANT_TX_POWER_USERAPP;
  
  sAntSetupData1.AntNetwork = ANT_NETWORK_DEFAULT;
  
  
  for(u8 i = 0; i < ANT_NETWORK_NUMBER_BYTES; i++)
  {
    sAntSetupData1.AntNetworkKey[i] = ANT_DEFAULT_NETWORK_KEY;
  }
    
  /* If good initialization, set state to Idle */
  if( AntAssignChannel(&sAntSetupData1) )
  {
    /* Channel assignment is queued so start timer */
#ifdef EIE1
    UserApp1_u32Timeout = G_u32SystemTime1ms;
    LedOn(RED);
#endif /* EIE1 */
    
    UserApp1_StateMachine = UserApp1SM_Wait0HiderChannelAssign;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */

    UserApp1_StateMachine = UserApp1SM_Error;
  }
}

static void UserApp1SM_Wait0HiderChannelAssign(void)
{
  /* Check if the channel assignment is complete */
  if(AntRadioStatusChannel(ANT_CHANNEL_USERAPP1) == ANT_CONFIGURED)
  {

    UserApp1_StateMachine = UserApp1SM_HiderIdle;
  }
  
  /* Monitor for timeout */
  if( IsTimeUp(&UserApp1_u32Timeout, 5000) )
  {
    DebugPrintf("\n\r***Channel assignment timeout***\n\n\r");
    UserApp1_StateMachine = UserApp1SM_Error;
  }
      
} 

/*Open the hider channel*/
static void UserApp1SM_HiderIdle(void)
{  
    AntOpenChannelNumber(ANT_CHANNEL_USERAPP1);
  
    UserApp1_u32Timeout = G_u32SystemTime1ms;
    UserApp1_StateMachine = UserApp1SM_WaitHiderChannelOpen;
 
} 

static void UserApp1SM_WaitHiderChannelOpen(void)
{
  /* Monitor the channel status to check if channel is opened */
  if(AntRadioStatusChannel(ANT_CHANNEL_USERAPP1) == ANT_OPEN )
  {   
    LedOn(RED);
    UserApp1_StateMachine = UserApp1SM_ChooseRole;
  }
  
  /* Check for timeout */
  if( IsTimeUp(&UserApp1_u32Timeout, TIMEOUT_VALUE) )
  {
    AntCloseChannelNumber(ANT_CHANNEL_USERAPP1);   
    
    UserApp1_StateMachine = UserApp1SM_HiderSet;
  }
    
} 

/*use this,you can choose your role when you begin the game*/
static void UserApp1SM_ChooseRole(void)
{
  static bool bRole = TRUE;

  
  if(bRole)
  {
    LCDCommand(LCD_CLEAR_CMD);
    LCDMessage(LINE1_START_ADDR, "Choose your role");
    LCDMessage(LINE2_START_ADDR, "B2:SEEKER B3:HIDER");
    bRole=FALSE;
  }
  UserApp1_StateMachine = UserApp1SM_ChannelOpen;
  
}

/* Channel is open, so monitor data */

static void UserApp1SM_ChannelOpen(void)
{
  static u8 u8LastState = 0xff;
  static u8 au8TickMessage[] = "EVENT x\n\r";  /* "x" at index [6] will be replaced by the current code */
  static u8 au8DataContent[] = "xxxxxxxxxxxxxxxx";
  static u8 au8LastAntData[ANT_APPLICATION_MESSAGE_BYTES] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  static u8 au8TestMessage[] = {0, 0, 0, 0, 0xA5, 0, 0, 0};
  static bool bRole = FALSE;
  static bool bStart = TRUE;
  static u32 u32Counter2=0;
  static u32 u32Counter1=0;
  static u32 u32Counter=0;
  static u8 u8Index = 10;
  static bool bState = FALSE;
  static u8 au8Time[] = "10";
  static bool bSeeker = FALSE;
  static bool bHider = FALSE;
  static bool bFinish = FALSE;
  static bool bFinish1 = FALSE;
  static bool bTime = FALSE;
  static bool bBuzzer = TRUE;
 
  bool bGotNewData;
  
  s8 s8Rssichannel0 = 0;
  
  /* Check for BUTTON0 to close channel */
  if(WasButtonPressed(BUTTON0))
  {
    /* Got the button, so complete one-time actions before next state */
    ButtonAcknowledge(BUTTON0);
    
    /* Queue close channel and change LED to blinking green to indicate channel is closing */
    /* Set timer and advance states */
    UserApp1_u32Timeout = G_u32SystemTime1ms;
    UserApp1_StateMachine = UserApp1SM_WaitChannelClose;
  } /* end if(WasButtonPressed(BUTTON0)) */
  
  /*Press button2 or button3,you can be a seeker or a hider*/
  if(WasButtonPressed(BUTTON2))
  {
    ButtonAcknowledge(BUTTON2);
    bRole = TRUE;
    bSeeker = TRUE;   
  }
  
  if(WasButtonPressed(BUTTON3))
  {
    ButtonAcknowledge(BUTTON3);
    bRole = FALSE;
    bHider = TRUE;
  }
  /*when you are a seeker, you will do these things */
  if(bRole)
  {
    if(bSeeker)
    {
      if(bBuzzer)
      {
        LedOff(ORANGE);
        LedOff(YELLOW);
        LedOff(GREEN);
        LedOff(CYAN);
        LedOff(BLUE);
        LedOff(PURPLE);
        LedOff(WHITE);
        LedOff(GREEN);
        LedOff(YELLOW);
        PWMAudioSetFrequency(BUZZER1,262);
        PWMAudioOn(BUZZER1); 
        bBuzzer = FALSE;
        u32Counter2 = 0;
      }
      u32Counter2++;
      if(u32Counter2 == 2000)
      {
              
        u32Counter2=0;
        bTime = TRUE;
      }
      if(bTime)
      {       
        PWMAudioOff(BUZZER1);
        u32Counter1++;
        if(bStart)
        {
          //PWMAudioOff(BUZZER1);
          LCDCommand(LCD_CLEAR_CMD);
          LCDMessage(LINE1_START_ADDR, "Seeker");
          LCDMessage(LINE2_START_ADDR, au8Time);
          bStart = FALSE;
        }
       /*the time counter and display it on lcd*/
        if(bState == FALSE)
        {       
          if(u32Counter1 % 1000 == 0  )
          {      
            u8Index = u8Index-1;
            au8Time[0]= '0';
            au8Time[1]= u8Index+'0';
            LCDCommand(LCD_CLEAR_CMD);
            LCDMessage(LINE1_START_ADDR, "Seeker");
            LCDMessage(LINE2_START_ADDR, au8Time);
            if(u8Index == 0)
            {
              u8Index = 10;
              u32Counter1 = 0;
              LCDCommand(LCD_CLEAR_CMD);
              LCDMessage(LINE1_START_ADDR, "Ready or not");
              LCDMessage(LINE2_START_ADDR, "Here I come!");
              bSeeker = FALSE;
              bState = TRUE;
              bFinish = TRUE;
              bTime = FALSE;
            }
          }  
        }
      }
    }
  }
  
  /*when you are a hider ,you will do these things*/
  if(bRole == FALSE)
  {
    if(bHider)
    {
      LedOff(ORANGE);
      LedOff(YELLOW);
      LedOff(GREEN);
      LedOff(CYAN);
      LedOff(BLUE);
      LedOff(PURPLE);
      LedOff(WHITE);
      LedOff(GREEN);
      LedOff(YELLOW);
      u32Counter++;
      PWMAudioSetFrequency(BUZZER1,262);
      PWMAudioOn(BUZZER1);
      if(u32Counter == 2000)
      {
        PWMAudioOff(BUZZER1);
        u32Counter = 0;
        bHider = FALSE;
        LCDCommand(LCD_CLEAR_CMD);
        LCDMessage(LINE1_START_ADDR, "Hider");
        bFinish1 = TRUE;
      }
    }
  }
  
  
  if( AntReadAppMessageBuffer() )
  {
     /* New data message: check what it is */
    if(G_eAntApiCurrentMessageClass == ANT_DATA)
    {
      UserApp1_u32DataMsgCount++;
      
      LedOff(RED);
      LedOff(ORANGE);
      LedOff(YELLOW);
      LedOff(GREEN);
      LedOff(CYAN);
      LedOff(BLUE);
      LedOff(PURPLE);
      LedOff(WHITE);
      LedOn(LCD_GREEN);
      LedOff(LCD_BLUE);
      LedOff(LCD_RED);
      /*Control the led by Rssi*/
      s8Rssichannel0 = G_sAntApiCurrentMessageExtData.s8RSSI;
      if(s8Rssichannel0 < -95)
      {
        LedOn(WHITE);
        PWMAudioSetFrequency(BUZZER1,50);
        PWMAudioOn(BUZZER1);
      }
      
      if(s8Rssichannel0 >= -95 && s8Rssichannel0 < -90)
      {
        LedOn(WHITE);
        LedOn(PURPLE);
        PWMAudioSetFrequency(BUZZER1,120);
        PWMAudioOn(BUZZER1);
      }
      
      if(s8Rssichannel0 >= -90 && s8Rssichannel0 < -85)
      {
        LedOn(WHITE);
        LedOn(PURPLE);
        LedOn(BLUE);
        PWMAudioSetFrequency(BUZZER1,190);
        PWMAudioOn(BUZZER1);
      }
      
      if(s8Rssichannel0 >= -85 && s8Rssichannel0 < -80)
      {
        LedOn(WHITE);
        LedOn(PURPLE);
        LedOn(BLUE);
        LedOn(CYAN);
      }
      
      if(s8Rssichannel0 >= -80 && s8Rssichannel0 < -75)
      {
        LedOn(WHITE);
        LedOn(PURPLE);
        LedOn(BLUE);
        LedOn(CYAN);
        LedOn(GREEN); 
        PWMAudioSetFrequency(BUZZER1,260);
        PWMAudioOn(BUZZER1);
      }
      
      if(s8Rssichannel0 >= -75 && s8Rssichannel0 < -70)
      {
        LedOn(WHITE);
        LedOn(PURPLE);
        LedOn(BLUE);
        LedOn(CYAN);
        LedOn(GREEN);  
        LedOn(YELLOW);
        PWMAudioSetFrequency(BUZZER1,330);
        PWMAudioOn(BUZZER1);
      }
      
      if(s8Rssichannel0 >= -70 && s8Rssichannel0 < -60)
      {
        LedOn(WHITE);
        LedOn(PURPLE);
        LedOn(BLUE);
        LedOn(CYAN);
        LedOn(GREEN);
        LedOn(YELLOW);
        LedOn(ORANGE);
        PWMAudioSetFrequency(BUZZER1,380);
        PWMAudioOn(BUZZER1);
      }
      
      if(s8Rssichannel0 >= -60 && s8Rssichannel0 <= -50)
      {       
        LedOn(WHITE);
        LedOn(PURPLE);
        LedOn(BLUE);
        LedOn(CYAN);
        LedOn(GREEN);
        LedOn(YELLOW);
        LedOn(ORANGE);
        LedOn(RED);
        PWMAudioOff(BUZZER1);
        /*check if the command from the seeker or the hider*/
        if(bFinish)
        {       
          if(bRole == TRUE)
          {
            bFinish = FALSE;
            LCDCommand(LCD_CLEAR_CMD);
            LCDMessage(LINE1_START_ADDR, "I Found YOU! ");
            bRole = FALSE;          
            bHider = TRUE; 
            bState = FALSE;
            bBuzzer = TRUE;
          }
        }
        
        if(bFinish1)
        {
          if(bRole == FALSE)
          {          
            bFinish1 = FALSE;
            LCDCommand(LCD_CLEAR_CMD);
            LCDMessage(LINE1_START_ADDR, "YOU Found ME! ");
            bSeeker = TRUE;
            bRole = TRUE;           
          }
        }
      }
      

      /* Check if the new data is the same as the old data and update as we go */
      bGotNewData = FALSE;
      for(u8 i = 0; i < ANT_APPLICATION_MESSAGE_BYTES; i++)
      {
        if(G_au8AntApiCurrentMessageBytes[i] != au8LastAntData[i])
        {
          bGotNewData = TRUE;
          au8LastAntData[i] = G_au8AntApiCurrentMessageBytes[i];

          au8DataContent[2 * i]     = HexToASCIICharUpper(G_au8AntApiCurrentMessageBytes[i] / 16);
          au8DataContent[2 * i + 1] = HexToASCIICharUpper(G_au8AntApiCurrentMessageBytes[i] % 16); 
        }
      }
      
      if(bGotNewData)
      {
        

        /* Check for a special packet and respond */
#ifdef MPG1
        if(G_au8AntApiCurrentMessageBytes[0] == 0xA5)
        {
          LedOff(LCD_RED);
          LedOff(LCD_GREEN);
          LedOff(LCD_BLUE);
          
          if(G_au8AntApiCurrentMessageBytes[1] == 1)
          {
            LedOn(LCD_RED);
          }
          
          if(G_au8AntApiCurrentMessageBytes[2] == 1)
          {
            LedOn(LCD_GREEN);
          }

          if(G_au8AntApiCurrentMessageBytes[3] == 1)
          {
            LedOn(LCD_BLUE);
          }
        }
#endif /* MPG1 */    
    

      } /* end if(bGotNewData) */
    } /* end if(G_eAntApiCurrentMessageClass == ANT_DATA) */
    
    else if(G_eAntApiCurrentMessageClass == ANT_TICK)
    {
      UserApp1_u32TickMsgCount++;

      /* Look at the TICK contents to check the event code and respond only if it's different */
      if(u8LastState != G_au8AntApiCurrentMessageBytes[ANT_TICK_MSG_EVENT_CODE_INDEX])
      {
        /* The state changed so update u8LastState and queue a debug message */
        u8LastState = G_au8AntApiCurrentMessageBytes[ANT_TICK_MSG_EVENT_CODE_INDEX];
        au8TickMessage[6] = HexToASCIICharUpper(u8LastState);
        DebugPrintf(au8TickMessage);

        /* Parse u8LastState to update LED status */
        switch (u8LastState)
        {
#ifdef MPG1
          /* If we are paired but missing messages, blue blinks */
          case EVENT_RX_FAIL:
          {
            LedOff(GREEN);
            LedBlink(BLUE, LED_2HZ);
            break;
          }

          /* If we drop to search, LED is green */
          case EVENT_RX_FAIL_GO_TO_SEARCH:
          {
            LedOff(BLUE);
            LedOn(GREEN);
            break;
          }
#endif /* MPG 1 */
#ifdef MPG2
          /* If we are paired but missing messages, blue blinks */
          case EVENT_RX_FAIL:
          {
            LedOff(GREEN0);
            LedBlink(BLUE0, LED_2HZ);
            break;
          }

          /* If we drop to search, LED is green */
          case EVENT_RX_FAIL_GO_TO_SEARCH:
          {
            LedOff(BLUE0);
            LedOn(GREEN0);
            break;
          }
#endif /* MPG 2 */
          /* If the search times out, the channel should automatically close */
          case EVENT_RX_SEARCH_TIMEOUT:
          {
            DebugPrintf("Search timeout event\r\n");
            break;
          }

          case EVENT_CHANNEL_CLOSED:
          {
            DebugPrintf("Channel closed event\r\n");
            break;
          }

            default:
          {
            DebugPrintf("Unexpected Event\r\n");
            break;
          }
        } /* end switch (G_au8AntApiCurrentMessageBytes) */
      } /* end if (u8LastState != G_au8AntApiCurrentMessageBytes[ANT_TICK_MSG_EVENT_CODE_INDEX]) */
    } /* end else if(G_eAntApiCurrentMessageClass == ANT_TICK) */
    
  } /* end AntReadAppMessageBuffer() */
  
  /* A slave channel can close on its own, so explicitly check channel status */
  if(AntRadioStatusChannel(ANT_CHANNEL_USERAPP) != ANT_OPEN)
  {
#ifdef MPG1
    LedBlink(GREEN, LED_2HZ);
    LedOff(BLUE);
#endif /* MPG1 */
    u8LastState = 0xff;
    
    UserApp1_u32Timeout = G_u32SystemTime1ms;
    UserApp1_StateMachine = UserApp1SM_WaitChannelClose;
  } /* if(AntRadioStatusChannel(ANT_CHANNEL_USERAPP) != ANT_OPEN) */
      
} /* end UserApp1SM_ChannelOpen() */




/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for channel to close */
static void UserApp1SM_WaitChannelClose(void)
{
  /* Monitor the channel status to check if channel is closed */
  if(AntRadioStatusChannel(ANT_CHANNEL_USERAPP) == ANT_CLOSED)
  {
    UserApp1_StateMachine = UserApp1SM_Idle;
  }
  
  /* Check for timeout */
  if( IsTimeUp(&UserApp1_u32Timeout, TIMEOUT_VALUE) )
  {
#ifdef MPG1   
    LedOff(ORANGE);
    LedOff(YELLOW);
    LedOff(GREEN);
    LedOff(CYAN);
    LedOff(BLUE);
    LedOff(PURPLE);
    LedOff(WHITE);
    LedOff(GREEN);
    LedOff(YELLOW);
    LedBlink(RED, LED_4HZ);
    LCDCommand(LCD_CLEAR_CMD);
    LCDMessage(LINE1_START_ADDR, "GAME OVER!");
#endif /* MPG1 */

#ifdef MPG2
    LedBlink(RED0, LED_4HZ);
    LedOff(GREEN0);
#endif /* MPG2 */
    
    UserApp1_StateMachine = UserApp1SM_Error;
  }
    
} /* end UserApp1SM_WaitChannelClose() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)          
{

} /* end UserApp1SM_Error() */




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
