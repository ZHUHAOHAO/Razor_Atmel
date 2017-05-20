/**********************************************************************************************************************
File: user_app1.c                                                                

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app1 as a template:
 1. Copy both user_app1.c and user_app1.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "user_app1" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "UserApp1" with "YourNewTaskName"
 6. Use ctrl-h to find and replace all instances of "USER_APP1" with "YOUR_NEW_TASK_NAME"
 7. Add a call to YourNewTaskNameInitialize() in the init section of main
 8. Add a call to YourNewTaskNameRunActiveState() in the Super Loop section of main
 9. Update yournewtaskname.h per the instructions at the top of yournewtaskname.h
10. Delete this text (between the dashed lines) and update the Description below to describe your task
----------------------------------------------------------------------------------------------------------------------

Description:
This is a user_app1.c file template 

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
All Global variable names shall start with "G_UserApp1"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp1Flags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp1_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp1_StateMachine;            /* The state machine function pointer */
//static u32 UserApp1_u32Timeout;                      /* Timeout counter used across states */


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
   LCDCommand(LCD_HOME_CMD);
   LCDCommand(LCD_CLEAR_CMD);

  /* If good initialization, set state to Idle */
  if( 1 )
  {
    UserApp1_StateMachine = UserApp1SM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp1_StateMachine = UserApp1SM_FailedInit;
  }

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
/* Wait for ??? */
static void UserApp1SM_Idle(void)
{
  static u8 u8Counter=LINE1_START_ADDR; 
  static u8 u8Counter1=0; 
  static u8 u8Counter2=0;
  u8 au8Message[]="group2"; 
  u8Counter1++; 
  static bool bJudge=TRUE;
  static bool bJudge1=TRUE;
  u8 u8Index;

  if(IsButtonPressed(BUTTON0)||IsButtonPressed(BUTTON1)||IsButtonPressed(BUTTON2)||IsButtonPressed(BUTTON3)) 
   { 
     PWMAudioSetFrequency(BUZZER1,262); 
     PWMAudioOn(BUZZER1); 
   } 
  else 
   {  
     PWMAudioOff(BUZZER1); 
   } 
  /*if any button pressed,the buzzer will be on*/ 
  if(WasButtonPressed(BUTTON0)) 
  { 
    ButtonAcknowledge(BUTTON0); 
    LedOn(LCD_RED); 
   } 
  if(WasButtonPressed(BUTTON1)) 
   { 
    ButtonAcknowledge(BUTTON1); 
    LedOn(LCD_BLUE); 
   } 
  if(WasButtonPressed(BUTTON2)) 
   { 
     ButtonAcknowledge(BUTTON2); 
     LedOn(LCD_GREEN); 
   } 
  if(WasButtonPressed(BUTTON3)) 
   { 
    ButtonAcknowledge(BUTTON3); 
    LedOff(LCD_RED); 
    LedOff(LCD_BLUE); 
    LedOff(LCD_GREEN); 
   } 
  /*Pressing the button and the LCD will change it's color*/ 
  if(u8Counter1==100) 
  { 
    LedOn(RED); 
    LedOn(WHITE); 
    LedOn(BLUE); 
    LedOn(YELLOW); 
  } 
  if(u8Counter1==200) 
  { 
    u8Counter1=0; 
    u8Counter++; 
    LedOff(RED); 
    LedOff(WHITE); 
    LedOff(BLUE); 
    LedOff(YELLOW); 
    LCDCommand(LCD_CLEAR_CMD); 
    for(u8Index=0;u8Index<sizeof(au8Message)-1;u8Index++)
    {
     if(u8Counter+u8Index==LINE1_END_ADDR+1)
     {
       bJudge=FALSE;
     }
     if(u8Counter+u8Index==LINE2_END_ADDR+1)
     {
       bJudge=FALSE;
       bJudge1=FALSE;
     }
     if(!bJudge)
     {
        if(!bJudge1)
        {
         u8Counter2=u8Counter+u8Index-LINE2_END_ADDR;       
         LCDMessage(LINE1_START_ADDR+u8Counter2,&au8Message[u8Index]);
        } 
        else
        {
         u8Counter2=u8Counter+u8Index-LINE1_END_ADDR;       
         LCDMessage(LINE2_START_ADDR+u8Counter2,&au8Message[u8Index]);
        }
     }
     else
     {
       LCDMessage(u8Counter+u8Index,&au8Message[u8Index]);
     }
     if(u8Index==sizeof(au8Message)-2)
     {
       bJudge=TRUE;
       bJudge1=TRUE;
     }
    }
  } 
  if(u8Counter==LINE1_END_ADDR) 
  { 
    u8Counter=LINE2_START_ADDR; 
  }   
  if(u8Counter==LINE2_END_ADDR) 
  { 
    u8Counter=LINE1_START_ADDR; 
  }
 /*Make your name move*/ 
} /* end UserApp1SM_Idle() */
    
#if 0
/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)          
{
  
} /* end UserApp1SM_Error() */
#endif


/*-------------------------------------------------------------------------------------------------------------------*/
/* State to sit in if init failed */
static void UserApp1SM_FailedInit(void)          
{
    
} /* end UserApp1SM_FailedInit() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
