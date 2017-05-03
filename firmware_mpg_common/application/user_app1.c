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
  LedOff(WHITE);
  LedOff(PURPLE);
  LedOff(BLUE);
  LedOff(CYAN);
  LedOff(GREEN);
  LedOff(YELLOW);
  LedOff(ORANGE);
  LedOff(RED);
  
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
 static u8 GetButtonValue(void)
 {
   u8 u8aButtonValue=0;
  if(WasButtonPressed(BUTTON0))
    {
      ButtonAcknowledge(BUTTON0);
      u8aButtonValue=1;
    }  
  if(WasButtonPressed(BUTTON1))
    {
      ButtonAcknowledge(BUTTON1);
      u8aButtonValue=2;
    }  
  if(WasButtonPressed(BUTTON2))
    {
      ButtonAcknowledge(BUTTON2);
      u8aButtonValue=3;
    }  
  if(WasButtonPressed(BUTTON3))
    {
      ButtonAcknowledge(BUTTON3);
      u8aButtonValue=4;
    }  
  return u8aButtonValue;
 }
/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for ??? */
static void UserApp1SM_Idle(void)
{ 
  static u8 u8aPasswords[100],u8aPasswords1[100];
  static u8 u8Counter=0,u8Counter1=0,u8GetNumber=0,u8GetNumber1=0;
  u8 u8Index;
  u8 u8Judge=1;
  static u8 u8JudgeNumber=0;
  
  if(IsButtonHeld(BUTTON3,1000))
    {
     LedOff(GREEN);
     LedOff(RED);
     LedBlink(GREEN,LED_2HZ);
     LedBlink(RED,LED_2HZ);
     u8JudgeNumber=1;
    }
  if(u8JudgeNumber==1)
    {
     u8GetNumber=GetButtonValue();
      if(u8GetNumber!=0)
       {
        if(u8GetNumber!=4)
        {
         u8aPasswords[u8Counter]=u8GetNumber;
         u8Counter++;
        }
        if(u8Counter!=0)
        {
         if(u8GetNumber==4)
         {
          LedOff(GREEN);
          LedOff(RED);
          LedOn(RED);
          u8JudgeNumber=2;
          u8Counter1=u8Counter;
          u8Counter=0;
         }
        }
       } 
    }
  if(u8JudgeNumber==2)
  {
   u8GetNumber1=GetButtonValue();
   if(u8GetNumber1!=0)
    {
      if(u8GetNumber1!=4)
      {
        u8aPasswords1[u8Counter]=u8GetNumber1;
        u8Counter++;
      }
    }
   
   if(u8GetNumber1==4)
    {
     if(u8Counter==u8Counter1) 
     {
       u8Counter=0;
       for(u8Index=0;u8Index<u8Counter1;u8Index++)
       {
        if(u8aPasswords1[u8Index]!=u8aPasswords[u8Index])
        {
          u8Judge=0;
          break;
        }
       }
     
       if(u8Judge)
       {    
        LedBlink(GREEN,LED_2HZ);
        LedOff(RED);
       }
       else
       {
        LedBlink(RED,LED_2HZ);
        LedOff(GREEN);
       }     
     }
     else
     {
      u8Counter=0;
      LedBlink(RED,LED_2HZ);
      LedOff(GREEN);
     }
    }
  }
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
