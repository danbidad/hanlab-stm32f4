#include "main.h"

//---------------------------------------------------

#define MAX_RPM             4500
#define MIN_RPM             300
#define SKIP_RPM_START      1100
#define SKIP_RPM_END        1500

#define MAX_RCF             3962
#define MIN_RCF             18
#define SKIP_RCF_START      237
#define SKIP_RCF_END        440

//---------------------------------------------------

struct SETTING gSetting;

//---------------------------------------------------

void SettingUI(void);
void ManageUI(void);
    static void EditOptions(void);
    static void GetConstant(void);
    static void EditVibConstant(void);
    static void EditPhaseConstant(void);
    static void EditLimitVib(void);
    static void EditLimitMass(void);
    static void EditBalancerWeight(void);
    static void EditBalancerDefaultPos(void);

//---------------------------------------------------

void GetConstant(void)
{
    GetConstantTask(0, false);

    strcpy(gPopup[0], "PRESS STOP OR ESC KEY");

	while ( 1 )
	{
        BackgroundProcess(false);
        UpdateMainUI(false);

        switch ( KEY_Get() )
        {
            case KEY_DOOR:
                DoorOpen();
                break;

            case KEY_STOP:
                return;
        }
    }
}

void SettingUI(void)
{
    char init_str[12];
    char temp_str[5];
    int temp;
    static struct PROGRAM program[20];
    int prg_idx = gProgramIdx;
    char* time_str;
    const static char* option_str_rpmrcf[2] = { "RPM", "RCF" };
    const static char* option_str_auto[2] = { "MANUAL", "AUTO" };

    FRAM_Read(200, &program, sizeof(program));

    /*
    for ( int i=0 ; i<20 ; i++ )
    {
        program[i].rpm = i*100 + 500;
        program[i].time = i*60;
    }
    */
restart:
    LCD_ClearScreen();
    FillLine2(0, 0, 1);
    LCD_PRINT(7, 0, "Setting", 0);

    // 메뉴 초기화
    {
        Menu_Init(1);

        Menu_Add_InputInteger("PROG NO.", 9, "%i", prg_idx, 0, 19, 1);

        Menu_Add("");

        Menu_Add_InputOption("RPM/RCF: ", 9, (char**)option_str_rpmrcf, program[prg_idx].rcf_mode, 0, 1, 3);

        Int2Str(init_str, program[prg_idx].rpm, 4, true);
        Menu_Add_InputDigit  ("", 9, "9999", init_str);

        sprintf(init_str, "%i:%s:%s", program[prg_idx].time/3600, Int2Str(temp_str, (program[prg_idx].time%3600)/60, 2, true), Int2Str(NULL, program[prg_idx].time%60, 2, true));
        Menu_Add_InputDigit  ("TIME", 9, "1:59:59", init_str);

        Menu_Add_InputInteger("ACCEL: ", 9, "%i", program[prg_idx].accel, 0, 9, 1);

        Menu_Add("");

        Menu_Add_InputOption("COOLER: ", 9, (char**)option_str_auto, program[prg_idx].cooler_auto_ctrl, 0, 1, 6);
        Menu_Add_InputInteger("TEMP:         'C", 9, "%i", program[prg_idx].temp, 10, 35, 1);

        Menu_Print(-1);
    }

	while ( 1 )
	{
        BackgroundProcess(false);

        switch ( KEY_Get() )
        {
#ifdef __AUTOMATIC_DOOR__
            case KEY_OPEN:
#ifdef __SB_VER_2__
                if ( DoorIsOpened() ) DoorClose();
                else DoorOpen();
#else
                DoorOpen();
#endif
                break;

            case KEY_CLOSE:
                DoorClose();
                break;
#endif
            case KEY_UP:
                if ( Menu.input_active )
                {
                    if ( Menu.focus_mid == 0 )
                    {
                        Input_GetValue(Menu.item[0].field_id, &prg_idx, NULL);

                        Input_GetValue(Menu.item[2].field_id, &temp, NULL);
                        program[prg_idx].rcf_mode = temp;

                        Input_GetValueStr(Menu.item[3].field_id, &time_str);
                        program[prg_idx].rpm = int_sscanf(time_str);

                        Input_GetValue(Menu.item[5].field_id, &program[prg_idx].accel, NULL);
                        Input_GetValueStr(Menu.item[4].field_id, &time_str);

                        program[prg_idx].time = int_sscanf(&time_str[0]) * 3600 +
                                                int_sscanf(&time_str[2]) * 60 +
                                                int_sscanf(&time_str[5]);


                        Input_GetValue(Menu.item[7].field_id, &temp, NULL);
                        program[prg_idx].cooler_auto_ctrl = temp;

                        Input_GetValue(Menu.item[8].field_id, &program[prg_idx].temp, NULL);
                    }
                    else if ( Menu.focus_mid == 2 ) // RPM/RCF
                    {
                        Input_GetValue(Menu.item[2].field_id, &temp, NULL);
                        program[prg_idx].rcf_mode = temp;

                        if ( temp == 0 ) // RPM -> RCF
                        {
                            Input_GetValueStr(Menu.item[3].field_id, &time_str);
                            temp = int_sscanf(time_str);
                            program[prg_idx].rpm = RPM2RCF(temp);
                            Int2Str(init_str, program[prg_idx].rpm, 4, true);
                            Input_SetValueDigit(Menu.item[3].field_id, init_str, true);
                        }
                    }

                    Input_ChangeValue(1);

                    if ( Menu.focus_mid == 0 )
                    {
                        if ( prg_idx < 19 ) prg_idx++;

                        Input_SetValueInt(Menu.item[2].field_id, program[prg_idx].rcf_mode, NULL, true);

                        Int2Str(init_str, program[prg_idx].rpm, 4, true);
                        Input_SetValueDigit(Menu.item[3].field_id, init_str, true);

                        sprintf(init_str, "%i:%s:%s", program[prg_idx].time/3600, Int2Str(temp_str, (program[prg_idx].time%3600)/60, 2, true), Int2Str(NULL, program[prg_idx].time%60, 2, true));

                        Input_SetValueInt(Menu.item[5].field_id, program[prg_idx].accel, NULL, true);
                        Input_SetValueDigit(Menu.item[4].field_id, init_str, true);

                        Input_SetValueInt(Menu.item[7].field_id, program[prg_idx].cooler_auto_ctrl, NULL, true);
                        Input_SetValueInt(Menu.item[8].field_id, program[prg_idx].temp, NULL, true);
                    }
                }
                else
                    Menu_Move(-1);
                break;
            case KEY_DOWN:
                if ( Menu.input_active )
                {
                    if ( Menu.focus_mid == 0 )
                    {
                        Input_GetValue(Menu.item[0].field_id, &prg_idx, NULL);

                        Input_GetValue(Menu.item[2].field_id, &temp, NULL);
                        program[prg_idx].rcf_mode = temp;

                        Input_GetValueStr(Menu.item[3].field_id, &time_str);
                        program[prg_idx].rpm = int_sscanf(time_str);

                        Input_GetValue(Menu.item[5].field_id, &program[prg_idx].accel, NULL);
                        Input_GetValueStr(Menu.item[4].field_id, &time_str);

                        program[prg_idx].time = int_sscanf(&time_str[0]) * 3600 +
                                                int_sscanf(&time_str[2]) * 60 +
                                                int_sscanf(&time_str[5]);

                        Input_GetValue(Menu.item[7].field_id, &temp, NULL);
                        program[prg_idx].cooler_auto_ctrl = temp;

                        Input_GetValue(Menu.item[8].field_id, &program[prg_idx].temp, NULL);
                    }
                    else if ( Menu.focus_mid == 2 ) // RPM/RCF
                    {
                        Input_GetValue(Menu.item[2].field_id, &temp, NULL);
                        program[prg_idx].rcf_mode = temp;

                        if ( temp == 1 ) // RCF -> RPM
                        {
                            Input_GetValueStr(Menu.item[3].field_id, &time_str);
                            temp = int_sscanf(time_str);
                            program[prg_idx].rpm = RCF2RPM(temp);
                            Int2Str(init_str, program[prg_idx].rpm, 4, true);
                            Input_SetValueDigit(Menu.item[3].field_id, init_str, true);
                        }
                    }

                    Input_ChangeValue(-1);

                    if ( Menu.focus_mid == 0 )
                    {
                        if ( prg_idx > 0 ) prg_idx--;

                        Input_SetValueInt(Menu.item[2].field_id, program[prg_idx].rcf_mode, NULL, true);

                        Int2Str(init_str, program[prg_idx].rpm, 4, true);
                        Input_SetValueDigit(Menu.item[3].field_id, init_str, true);

                        sprintf(init_str, "%i:%s:%s", program[prg_idx].time/3600, Int2Str(temp_str, (program[prg_idx].time%3600)/60, 2, true), Int2Str(NULL, program[prg_idx].time%60, 2, true));
                        Input_SetValueInt(Menu.item[5].field_id, program[prg_idx].accel, NULL, true);
                        Input_SetValueDigit(Menu.item[4].field_id, init_str, true);
                        Input_SetValueInt(Menu.item[7].field_id, program[prg_idx].cooler_auto_ctrl, NULL, true);
                        Input_SetValueInt(Menu.item[8].field_id, program[prg_idx].temp, NULL, true);
                    }
                }
                else
                    Menu_Move(1);
                break;
            case KEY_LEFT:
                if ( Menu.input_active )
                    Input_MoveByCursor(-1);
                break;
            case KEY_RIGHT:
                if ( Menu.input_active )
                    Input_MoveByCursor(1);
                break;
            case KEY_ENTER:
                if ( Menu.focus_mid == 3 && Menu.input_active )
                {
                    Input_GetValue(Menu.item[2].field_id, &temp, NULL);

                    if ( temp ) // RCF
                    {
                        Input_GetValueStr(Menu.item[3].field_id, &time_str);
                        temp = int_sscanf(time_str);

                        if ( temp > MAX_RCF ) temp = MAX_RCF;
                        else if ( temp < MIN_RCF ) temp = MIN_RCF;
                        else if ( temp > SKIP_RCF_START && temp < (SKIP_RCF_START+SKIP_RCF_END)/2 ) temp = SKIP_RCF_START;
                        else if ( temp < SKIP_RCF_END && temp >= (SKIP_RCF_START+SKIP_RCF_END)/2 ) temp = SKIP_RCF_END;
                    }
                    else // RPM
                    {
                        Input_GetValueStr(Menu.item[3].field_id, &time_str);
                        temp = int_sscanf(time_str);

                        if ( temp > MAX_RPM ) temp = MAX_RPM;
                        else if ( temp < MIN_RPM ) temp = MIN_RPM;
                        else if ( temp > SKIP_RPM_START && temp < (SKIP_RPM_START+SKIP_RPM_END)/2 ) temp = SKIP_RPM_START;
                        else if ( temp < SKIP_RPM_END && temp >= (SKIP_RPM_START+SKIP_RPM_END)/2 ) temp = SKIP_RPM_END;
                    }

                    program[prg_idx].rpm = temp;
                    Int2Str(init_str, program[prg_idx].rpm, 4, true);
                    Input_SetValueDigit(Menu.item[3].field_id, init_str, true);

                    if ( program[prg_idx].rcf_mode )
                        temp = RCF2RPM(program[prg_idx].rpm);
                    else
                        temp = program[prg_idx].rpm;

                    if ( temp >= 4400 && program[prg_idx].time > 3600 )
                    {
                        program[prg_idx].time = 3600;
                        sprintf(init_str, "%i:%s:%s", program[prg_idx].time/3600, Int2Str(temp_str, (program[prg_idx].time%3600)/60, 2, true), Int2Str(NULL, program[prg_idx].time%60, 2, true));
                        Input_SetValueDigit(Menu.item[4].field_id, init_str, true);
                    }
                }
                else if ( Menu.focus_mid == 4 && Menu.input_active )
                {
                    Input_GetValueStr(Menu.item[4].field_id, &time_str);
                    program[prg_idx].time = int_sscanf(&time_str[0]) * 3600 +
                                            int_sscanf(&time_str[2]) * 60 +
                                            int_sscanf(&time_str[5]);

                    if ( program[prg_idx].time == 0 )
                    {
                        program[prg_idx].time = 300;
                        sprintf(init_str, "%i:%s:%s", program[prg_idx].time/3600, Int2Str(temp_str, (program[prg_idx].time%3600)/60, 2, true), Int2Str(NULL, program[prg_idx].time%60, 2, true));
                        Input_SetValueDigit(Menu.item[4].field_id, init_str, true);
                    }

                    if ( program[prg_idx].rcf_mode )
                        temp = RCF2RPM(program[prg_idx].rpm);
                    else
                        temp = program[prg_idx].rpm;

                    if ( temp >= 4400 && program[prg_idx].time > 3600 )
                    {
                        program[prg_idx].time = 3600;
                        sprintf(init_str, "%i:%s:%s", program[prg_idx].time/3600, Int2Str(temp_str, (program[prg_idx].time%3600)/60, 2, true), Int2Str(NULL, program[prg_idx].time%60, 2, true));
                        Input_SetValueDigit(Menu.item[4].field_id, init_str, true);
                    }
                }
                Menu_InputSwitch();
                break;

            case KEY_SAVE:
                {
                    Input_GetValue(Menu.item[0].field_id, &prg_idx, NULL);

                    Input_GetValue(Menu.item[2].field_id, &temp, NULL);
                    program[prg_idx].rcf_mode = temp;

                    Input_GetValueStr(Menu.item[3].field_id, &time_str);
                    program[prg_idx].rpm = int_sscanf(time_str);

                    if ( program[prg_idx].rcf_mode ) // RCF
                    {
                        temp = program[prg_idx].rpm;

                        if ( temp > MAX_RCF ) temp = MAX_RCF;
                        else if ( temp < MIN_RCF ) temp = MIN_RCF;
                        else if ( temp > SKIP_RCF_START && temp < (SKIP_RCF_START+SKIP_RCF_END)/2 ) temp = SKIP_RCF_START;
                        else if ( temp < SKIP_RCF_END && temp >= (SKIP_RCF_START+SKIP_RCF_END)/2 ) temp = SKIP_RCF_END;

                        program[prg_idx].rpm = temp;

                    }
                    else // RPM
                    {
                        temp = program[prg_idx].rpm;

                        if ( temp > MAX_RPM ) temp = MAX_RPM;
                        else if ( temp < MIN_RPM ) temp = MIN_RPM;
                        else if ( temp > SKIP_RPM_START && temp < (SKIP_RPM_START+SKIP_RPM_END)/2 ) temp = SKIP_RPM_START;
                        else if ( temp < SKIP_RPM_END && temp >= (SKIP_RPM_START+SKIP_RPM_END)/2 ) temp = SKIP_RPM_END;

                        program[prg_idx].rpm = temp;
                    }

                    Input_GetValue(Menu.item[5].field_id, &program[prg_idx].accel, NULL);
                    Input_GetValueStr(Menu.item[4].field_id, &time_str);
                    program[prg_idx].time = int_sscanf(&time_str[0]) * 3600 +
                                            int_sscanf(&time_str[2]) * 60 +
                                            int_sscanf(&time_str[5]);

                    if ( program[prg_idx].time == 0 )
                        program[prg_idx].time = 300;

                    if ( program[prg_idx].rcf_mode )
                        temp = RCF2RPM(program[prg_idx].rpm);
                    else
                        temp = program[prg_idx].rpm;

                    if ( temp >= 4400 && program[prg_idx].time > 3600 )
                        program[prg_idx].time = 3600;

                    Input_GetValue(Menu.item[7].field_id, &temp, NULL);
                    program[prg_idx].cooler_auto_ctrl = temp;

                    Input_GetValue(Menu.item[8].field_id, &program[prg_idx].temp, NULL);

                    FRAM_Write(196, &prg_idx, sizeof(prg_idx));
                    FRAM_Write(200, &program, sizeof(program));

                    gProgramIdx = prg_idx;
                    FRAM_Read(200, &gProgram, sizeof(gProgram));

                    gCoolerAutoControl = gProgram[gProgramIdx].cooler_auto_ctrl;
                    gCoolerTemp = gProgram[gProgramIdx].temp;
                    gFanAutoControl = true;

                    if ( !gCoolerAutoControl )
                        CoolerControl(false);
                }
            case KEY_STOP:
                strcpy(gPopup[0], "READY");
                gPopup[1][0] = NULL;
                return;
        }
    }
}

void ManageUI(void)
{
restart:
    LCD_ClearScreen();
    FillLine2(0, 0, 1);
    LCD_PRINT(2, 0, "Administrator Menu", 0);

    Menu_Init(1);
    Menu_Add("1. Edit Options");
    Menu_Add("2. Measure Const");
    Menu_Add("3. Edit Vib Const");
    Menu_Add("4. Edit Phase Const");
    Menu_Add("5. Edit Limit Vib");
    Menu_Add("6. Edit Limit Mass");
    Menu_Add("7. Balancer Weight");
    Menu_Add("8. Balancer Def Pos");
    Menu_Print(-1);

    while ( 1 )
    {
        BackgroundProcess(false);

        switch ( KEY_Get() )
        {
#ifdef __AUTOMATIC_DOOR__
            case KEY_OPEN:
#ifdef __SB_VER_2__
                if ( DoorIsOpened() ) DoorClose();
                else DoorOpen();
#else
                DoorOpen();
#endif
                break;

            case KEY_CLOSE:
                DoorClose();
                break;
#endif
            case KEY_UP:
                Menu_Move(-1);
                break;

            case KEY_DOWN:
                Menu_Move(1);
                break;

            case KEY_STOP:
            case KEY_SAVE:
                strcpy(gPopup[0], "READY");
                gPopup[1][0] = NULL;
                return;

            case KEY_ENTER:
                switch ( Menu.focus_mid )
                {
                    case 0:
                        EditOptions();
                        goto restart;
                    case 1:
                        GetConstant();
                        goto restart;
                    case 2:
                        EditVibConstant();
                        goto restart;
                    case 3:
                        EditPhaseConstant();
                        goto restart;
                    case 4:
                        EditLimitVib();
                        goto restart;
                    case 5:
                        EditLimitMass();
                        goto restart;
                    case 6:
                        EditBalancerWeight();
                        goto restart;
                    case 7:
                        EditBalancerDefaultPos();
                        goto restart;
                }
                break;
        }
    }
}


void EditVibConstant(void)
{
    char init_str[12];
    struct SwingRotorBalancingParam swing_rotor_param;

    FRAM_Read(100, &swing_rotor_param, sizeof(swing_rotor_param));

    LCD_ClearScreen();
    FillLine2(0, 0, 1);
    LCD_PRINT(4, 0, "Edit Vib Const", 0);

    // 메뉴 초기화
    {
        Menu_Init(1);

        float_sprintf(init_str, swing_rotor_param.vib_const[0], 1, true, 6);
        WS(init_str);
        Menu_Add("VibConst at " __SWING_ROTOR_300_RPM_STR__ "rpm:");
        Menu_Add_InputDigit  ("", 13, "0.999999", init_str);

        float_sprintf(init_str, swing_rotor_param.vib_const[1], 1, true, 6);
        WS(init_str);
        Menu_Add("VibConst at " __SWING_ROTOR_1000_RPM_STR__ "rpm:");
        Menu_Add_InputDigit  ("", 13, "0.999999", init_str);

        float_sprintf(init_str, swing_rotor_param.vib_const[2], 1, true, 6);
        WS(init_str);
        Menu_Add("VibConst at " __SWING_ROTOR_2000_RPM_STR__ "rpm:");
        Menu_Add_InputDigit  ("", 13, "0.999999", init_str);

        Menu_Print(-1);
    }

	while ( 1 )
	{
        BackgroundProcess(false);

        switch ( KEY_Get() )
        {
#ifdef __AUTOMATIC_DOOR__
            case KEY_OPEN:
#ifdef __SB_VER_2__
                if ( DoorIsOpened() ) DoorClose();
                else DoorOpen();
#else
                DoorOpen();
#endif
                break;

            case KEY_CLOSE:
                DoorClose();
                break;
#endif
            case KEY_UP:
                if ( Menu.input_active )
                    Input_ChangeValue(1);
                else
                    Menu_Move(-1);
                break;
            case KEY_DOWN:
                if ( Menu.input_active )
                    Input_ChangeValue(-1);
                else
                    Menu_Move(1);
                break;
            case KEY_LEFT:
                if ( Menu.input_active )
                    Input_MoveByCursor(-1);
                break;
            case KEY_RIGHT:
                if ( Menu.input_active )
                    Input_MoveByCursor(1);
                break;
            case KEY_ENTER:
                Menu_InputSwitch();
                break;
            case KEY_SAVE:
                {
                    float temp_vib_const[3];

                    Input_GetValue(Menu.item[1].field_id, NULL, &temp_vib_const[0]);
                    Input_GetValue(Menu.item[3].field_id, NULL, &temp_vib_const[1]);
                    Input_GetValue(Menu.item[5].field_id, NULL, &temp_vib_const[2]);

                    swing_rotor_param.vib_const[0] = temp_vib_const[0];
                    swing_rotor_param.vib_const[1] = temp_vib_const[1];
                    swing_rotor_param.vib_const[2] = temp_vib_const[2];

                    if ( swing_rotor_param.vib_const[0]!=0 && swing_rotor_param.limit_mass_at_rpm[0]!=0 ) swing_rotor_param.limit_vib_at_rpm[0] = swing_rotor_param.limit_mass_at_rpm[0] * ROTOR_RADIUS_FOR_VIB_LIMIT * swing_rotor_param.vib_const[0];
                    if ( swing_rotor_param.vib_const[1]!=0 && swing_rotor_param.limit_mass_at_rpm[0]!=0 ) swing_rotor_param.limit_vib_at_rpm[1] = swing_rotor_param.limit_mass_at_rpm[1] * ROTOR_RADIUS_FOR_VIB_LIMIT * swing_rotor_param.vib_const[1];
                    if ( swing_rotor_param.vib_const[2]!=0 && swing_rotor_param.limit_mass_at_rpm[0]!=0 ) swing_rotor_param.limit_vib_at_rpm[2] = swing_rotor_param.limit_mass_at_rpm[2] * ROTOR_RADIUS_FOR_VIB_LIMIT * swing_rotor_param.vib_const[2];

                    gSwingRotorBalancingParam = swing_rotor_param;
                    FRAM_Write(100, &swing_rotor_param, sizeof(swing_rotor_param));
                }
            case KEY_STOP:
                return;
        }
    }
}


void EditPhaseConstant(void)
{
    char init_str[12];
    struct SwingRotorBalancingParam swing_rotor_param;

    FRAM_Read(100, &swing_rotor_param, sizeof(swing_rotor_param));

    while ( swing_rotor_param.phase_const[0] < 0 )
        swing_rotor_param.phase_const[0] += 360;

    while ( swing_rotor_param.phase_const[1] < 0 )
        swing_rotor_param.phase_const[1] += 360;

    while ( swing_rotor_param.phase_const[2] < 0 )
        swing_rotor_param.phase_const[2] += 360;

    LCD_ClearScreen();
    FillLine2(0, 0, 1);
    LCD_PRINT(3, 0, "Edit Phase Const", 0);

    // 메뉴 초기화
    {
        Menu_Init(1);

        float_sprintf(init_str, swing_rotor_param.phase_const[0], 3, true, 6);
        Menu_Add("PhaseConst at " __SWING_ROTOR_300_RPM_STR__ "rpm");
        Menu_Add_InputDigit  ("", 11, "399.999999", init_str);

        float_sprintf(init_str, swing_rotor_param.phase_const[1], 3, true, 6);
        Menu_Add("PhaseConst at " __SWING_ROTOR_1000_RPM_STR__ "rpm");
        Menu_Add_InputDigit  ("", 11, "399.999999", init_str);

        float_sprintf(init_str, swing_rotor_param.phase_const[2], 3, true, 6);
        Menu_Add("PhaseConst at " __SWING_ROTOR_2000_RPM_STR__ "rpm");
        Menu_Add_InputDigit  ("", 11, "399.999999", init_str);

        Menu_Print(-1);
    }

	while ( 1 )
	{
        BackgroundProcess(false);

        switch ( KEY_Get() )
        {
#ifdef __AUTOMATIC_DOOR__
            case KEY_OPEN:
#ifdef __SB_VER_2__
                if ( DoorIsOpened() ) DoorClose();
                else DoorOpen();
#else
                DoorOpen();
#endif
                break;

            case KEY_CLOSE:
                DoorClose();
                break;
#endif
            case KEY_UP:
                if ( Menu.input_active )
                    Input_ChangeValue(1);
                else
                    Menu_Move(-1);
                break;
            case KEY_DOWN:
                if ( Menu.input_active )
                    Input_ChangeValue(-1);
                else
                    Menu_Move(1);
                break;
            case KEY_LEFT:
                if ( Menu.input_active )
                    Input_MoveByCursor(-1);
                break;
            case KEY_RIGHT:
                if ( Menu.input_active )
                    Input_MoveByCursor(1);
                break;
            case KEY_ENTER:
                Menu_InputSwitch();
                break;
            case KEY_SAVE:
                {
                    float temp_phase_const[3];

                    Input_GetValue(Menu.item[1].field_id, NULL, &temp_phase_const[0]);
                    Input_GetValue(Menu.item[3].field_id, NULL, &temp_phase_const[1]);
                    Input_GetValue(Menu.item[5].field_id, NULL, &temp_phase_const[2]);

                    swing_rotor_param.phase_const[0] = temp_phase_const[0];
                    swing_rotor_param.phase_const[1] = temp_phase_const[1];
                    swing_rotor_param.phase_const[2] = temp_phase_const[2];

                    if ( swing_rotor_param.phase_const[0] > 180 ) swing_rotor_param.phase_const[0] -= 360;
                    if ( swing_rotor_param.phase_const[1] > 180 ) swing_rotor_param.phase_const[1] -= 360;
                    if ( swing_rotor_param.phase_const[2] > 180 ) swing_rotor_param.phase_const[2] -= 360;

                    gSwingRotorBalancingParam = swing_rotor_param;
                    FRAM_Write(100, &swing_rotor_param, sizeof(swing_rotor_param));
                }
            case KEY_STOP:
                return;
        }
    }
}

void EditLimitVib(void)
{
    char init_str[11];
    struct SwingRotorBalancingParam swing_rotor_param;

    FRAM_Read(100, &swing_rotor_param, sizeof(swing_rotor_param));

    LCD_ClearScreen();
    FillLine2(0, 0, 1);
    LCD_PRINT(4, 0, "Edit Limit Vib", 0);

    // 메뉴 초기화
    {
        Menu_Init(1);

        float_sprintf(init_str, swing_rotor_param.limit_vib_at_rpm[0], 2, true, 6);
        Menu_Add("Limit Vib at " __SWING_ROTOR_300_RPM_STR__ "rpm:");
        Menu_Add_InputDigit("", 12, "99.999999", init_str);

        float_sprintf(init_str, swing_rotor_param.limit_vib_at_rpm[1], 2, true, 6);
        Menu_Add("Limit Vib at " __SWING_ROTOR_1000_RPM_STR__ "rpm:");
        Menu_Add_InputDigit  ("", 12, "99.999999", init_str);

        float_sprintf(init_str, swing_rotor_param.limit_vib_at_rpm[2], 3, true, 6);
        Menu_Add("Limit Vib at " __SWING_ROTOR_2000_RPM_STR__ "rpm:");
        Menu_Add_InputDigit  ("", 11, "999.999999", init_str);

        Menu_Print(-1);
    }

	while ( 1 )
	{
        BackgroundProcess(false);

        switch ( KEY_Get() )
        {
#ifdef __AUTOMATIC_DOOR__
            case KEY_OPEN:
#ifdef __SB_VER_2__
                if ( DoorIsOpened() ) DoorClose();
                else DoorOpen();
#else
                DoorOpen();
#endif
                break;

            case KEY_CLOSE:
                DoorClose();
                break;
#endif
            case KEY_UP:
                if ( Menu.input_active )
                    Input_ChangeValue(1);
                else
                    Menu_Move(-1);
                break;
            case KEY_DOWN:
                if ( Menu.input_active )
                    Input_ChangeValue(-1);
                else
                    Menu_Move(1);
                break;
            case KEY_LEFT:
                if ( Menu.input_active )
                    Input_MoveByCursor(-1);
                break;
            case KEY_RIGHT:
                if ( Menu.input_active )
                    Input_MoveByCursor(1);
                break;
            case KEY_ENTER:
                Menu_InputSwitch();
                break;
            case KEY_SAVE:
                {
                    Input_GetValue(Menu.item[1].field_id, NULL, &swing_rotor_param.limit_vib_at_rpm[0]);
                    Input_GetValue(Menu.item[3].field_id, NULL, &swing_rotor_param.limit_vib_at_rpm[1]);
                    Input_GetValue(Menu.item[5].field_id, NULL, &swing_rotor_param.limit_vib_at_rpm[2]);

                    swing_rotor_param.limit_mass_at_rpm[0] = swing_rotor_param.limit_vib_at_rpm[0] / (ROTOR_RADIUS_FOR_VIB_LIMIT * swing_rotor_param.vib_const[0]);
                    swing_rotor_param.limit_mass_at_rpm[1] = swing_rotor_param.limit_vib_at_rpm[1] / (ROTOR_RADIUS_FOR_VIB_LIMIT * swing_rotor_param.vib_const[1]);
                    swing_rotor_param.limit_mass_at_rpm[2] = swing_rotor_param.limit_vib_at_rpm[2] / (ROTOR_RADIUS_FOR_VIB_LIMIT * swing_rotor_param.vib_const[2]);

                    gSwingRotorBalancingParam = swing_rotor_param;
                    FRAM_Write(100, &swing_rotor_param, sizeof(swing_rotor_param));
                }
            case KEY_STOP:
                return;
        }
    }
}

void EditLimitMass(void)
{
    char init_str[11];
    struct SwingRotorBalancingParam swing_rotor_param;

    FRAM_Read(100, &swing_rotor_param, sizeof(swing_rotor_param));

    LCD_ClearScreen();
    FillLine2(0, 0, 1);
    LCD_PRINT(3, 0, "Edit Limit Mass", 0);

    // 메뉴 초기화
    {
        Menu_Init(1);

        float_sprintf(init_str, swing_rotor_param.limit_mass_at_rpm[0], 2, true, 6);
        Menu_Add("Limit Mass at " __SWING_ROTOR_300_RPM_STR__ "rpm:");
        Menu_Add_InputDigit  ("", 12, "69.999999", init_str);

        float_sprintf(init_str, swing_rotor_param.limit_mass_at_rpm[1], 2, true, 6);
        Menu_Add("Limit Mass at " __SWING_ROTOR_1000_RPM_STR__ "rpm:");
        Menu_Add_InputDigit  ("", 12, "69.999999", init_str);

        float_sprintf(init_str, swing_rotor_param.limit_mass_at_rpm[2], 2, true, 6);
        Menu_Add("Limit Mass at " __SWING_ROTOR_2000_RPM_STR__ "rpm:");
        Menu_Add_InputDigit  ("", 12, "69.999999", init_str);

        Menu_Print(-1);
    }

	while ( 1 )
	{
        BackgroundProcess(false);

        switch ( KEY_Get() )
        {
#ifdef __AUTOMATIC_DOOR__
            case KEY_OPEN:
#ifdef __SB_VER_2__
                if ( DoorIsOpened() ) DoorClose();
                else DoorOpen();
#else
                DoorOpen();
#endif
                break;

            case KEY_CLOSE:
                DoorClose();
                break;
#endif
            case KEY_UP:
                if ( Menu.input_active )
                    Input_ChangeValue(1);
                else
                    Menu_Move(-1);
                break;
            case KEY_DOWN:
                if ( Menu.input_active )
                    Input_ChangeValue(-1);
                else
                    Menu_Move(1);
                break;
            case KEY_LEFT:
                if ( Menu.input_active )
                    Input_MoveByCursor(-1);
                break;
            case KEY_RIGHT:
                if ( Menu.input_active )
                    Input_MoveByCursor(1);
                break;
            case KEY_ENTER:
                Menu_InputSwitch();
                break;
            case KEY_SAVE:
                {
                    Input_GetValue(Menu.item[1].field_id, NULL, &swing_rotor_param.limit_mass_at_rpm[0]);
                    Input_GetValue(Menu.item[3].field_id, NULL, &swing_rotor_param.limit_mass_at_rpm[1]);
                    Input_GetValue(Menu.item[5].field_id, NULL, &swing_rotor_param.limit_mass_at_rpm[2]);

                    swing_rotor_param.limit_vib_at_rpm[0] = swing_rotor_param.limit_mass_at_rpm[0] * ROTOR_RADIUS_FOR_VIB_LIMIT * swing_rotor_param.vib_const[0];
                    swing_rotor_param.limit_vib_at_rpm[1] = swing_rotor_param.limit_mass_at_rpm[1] * ROTOR_RADIUS_FOR_VIB_LIMIT * swing_rotor_param.vib_const[1];
                    swing_rotor_param.limit_vib_at_rpm[2] = swing_rotor_param.limit_mass_at_rpm[2] * ROTOR_RADIUS_FOR_VIB_LIMIT * swing_rotor_param.vib_const[2];

                    gSwingRotorBalancingParam = swing_rotor_param;
                    FRAM_Write(100, &swing_rotor_param, sizeof(swing_rotor_param));
                }
            case KEY_STOP:
                return;
        }
    }
}

void EditBalancerWeight(void)
{
    LCD_ClearScreen();
    FillLine2(0, 0, 1);
    LCD_PRINT(1, 0, "Edit Balancer Weight", 0);

    RB_PowerOnAndCheck(0);
    while ( gRotor.status != ROTOR_READY ) BackgroundProcess(true);

    if ( gRotor.result != SUCCESS )
    {
        RotorPower(0);
        SlipRingPower(0);
        Buzzer(2500, 200);
        return;
    }

    RB_GetInformation();
    while ( gRotor.status != ROTOR_FINISHED ) BackgroundProcess(true);

    RotorPower(0);
    SlipRingPower(0);

    if ( gRotor.result != SUCCESS )
    {
        Buzzer(2500, 200);
        return;
    }
    else
    {
        Buzzer(1000, 30);
        Buzzer(1500, 30);
        Buzzer(2000, 30);
    }

    // 메뉴 초기화
    {
        Menu_Init(1);

        Menu_Add("Read");
        Menu_Add("Write");
        Menu_Add("");

        Menu_Add_InputDigit  ("Bal 1 weight:", 14, "9999", Int2Str(NULL, gRotor.weight[0], 4, true));
        Menu_Add_InputDigit  ("Bal 2 weight:", 14, "9999", Int2Str(NULL, gRotor.weight[1], 4, true));
        Menu_Add_InputDigit  ("Bal 3 weight:", 14, "9999", Int2Str(NULL, gRotor.weight[2], 4, true));

        Menu_Print(-1);
    }

	while ( 1 )
	{
        BackgroundProcess(false);

        switch ( KEY_Get() )
        {
#ifdef __AUTOMATIC_DOOR__
            case KEY_OPEN:
#ifdef __SB_VER_2__
                if ( DoorIsOpened() ) DoorClose();
                else DoorOpen();
#else
                DoorOpen();
#endif
                break;

            case KEY_CLOSE:
                DoorClose();
                break;
#endif
            case KEY_UP:
                if ( Menu.input_active )
                    Input_ChangeValue(1);
                else
                    Menu_Move(-1);
                break;
            case KEY_DOWN:
                if ( Menu.input_active )
                    Input_ChangeValue(-1);
                else
                    Menu_Move(1);
                break;
            case KEY_LEFT:
                if ( Menu.input_active )
                    Input_MoveByCursor(-1);
                break;
            case KEY_RIGHT:
                if ( Menu.input_active )
                    Input_MoveByCursor(1);
                break;
            case KEY_ENTER:
                Menu_InputSwitch();
                switch ( Menu.focus_mid )
                {
                    case 0:
                        RB_PowerOnAndCheck(0);
                        while ( gRotor.status != ROTOR_READY ) BackgroundProcess(true);

                        if ( gRotor.result != SUCCESS )
                        {
                            RotorPower(0);
                            SlipRingPower(0);
                            Buzzer(2500, 200);
                            break;
                        }

                        RB_GetInformation();
                        while ( gRotor.status != ROTOR_FINISHED ) BackgroundProcess(true);

                        RotorPower(0);
                        SlipRingPower(0);

                        if ( gRotor.result != SUCCESS )
                        {
                            Buzzer(2500, 200);
                        }
                        else
                        {
                            Input_SetValueDigit(Menu.item[3].field_id, Int2Str(NULL, gRotor.weight[0], 4, true), true);
                            Input_SetValueDigit(Menu.item[4].field_id, Int2Str(NULL, gRotor.weight[1], 4, true), true);
                            Input_SetValueDigit(Menu.item[5].field_id, Int2Str(NULL, gRotor.weight[2], 4, true), true);

                            Buzzer(1000, 30);
                            Buzzer(1500, 30);
                            Buzzer(2000, 30);
                        }
                        break;
                    case 1:
                        goto save_process;
                }
                break;

            case KEY_SAVE:
save_process:
                Input_GetValue(Menu.item[3].field_id, &gRotor.weight[0], NULL);
                Input_GetValue(Menu.item[4].field_id, &gRotor.weight[1], NULL);
                Input_GetValue(Menu.item[5].field_id, &gRotor.weight[2], NULL);

                RB_PowerOnAndCheck(0);
                while ( gRotor.status != ROTOR_READY ) BackgroundProcess(true);

                if ( gRotor.result != SUCCESS )
                {
                    RotorPower(0);
                    SlipRingPower(0);
                    Buzzer(2500, 200);
                    break;;
                }

                RB_SetBalancerWeight(gRotor.weight[0], gRotor.weight[1], gRotor.weight[2]);
                while ( gRotor.status != ROTOR_FINISHED ) BackgroundProcess(true);

                RotorPower(0);
                SlipRingPower(0);

                if ( gRotor.result != SUCCESS )
                {
                    Buzzer(2500, 200);
                }
                else
                {
                    Buzzer(1000, 30);
                    Buzzer(1500, 30);
                    Buzzer(2000, 30);
                }
                break;

            case KEY_STOP:
                return;
        }
    }
}


void EditBalancerDefaultPos(void)
{
    LCD_ClearScreen();
    FillLine2(0, 0, 1);
    LCD_PRINT(0, 0, "Edit Balancer Def Pos", 0);

    RB_PowerOnAndCheck(0);
    while ( gRotor.status != ROTOR_READY ) BackgroundProcess(true);

    if ( gRotor.result != SUCCESS )
    {
        RotorPower(0);
        SlipRingPower(0);
        Buzzer(2500, 200);
        return;
    }

    RB_GetInformation();
    while ( gRotor.status != ROTOR_FINISHED ) BackgroundProcess(true);

    RotorPower(0);
    SlipRingPower(0);

    if ( gRotor.result != SUCCESS )
    {
        Buzzer(2500, 200);
        return;
    }
    else
    {
        Buzzer(1000, 30);
        Buzzer(1500, 30);
        Buzzer(2000, 30);
    }

    // 메뉴 초기화
    {
        Menu_Init(1);

        Menu_Add("Read");
        Menu_Add("Write");
        Menu_Add("");

        Menu_Add_InputDigit  ("Bal 1 def pos:", 15, "9999", Int2Str(NULL, gRotor.default_pos[0], 4, true));
        Menu_Add_InputDigit  ("Bal 2 def pos:", 15, "9999", Int2Str(NULL, gRotor.default_pos[1], 4, true));
        Menu_Add_InputDigit  ("Bal 3 def pos:", 15, "9999", Int2Str(NULL, gRotor.default_pos[2], 4, true));

        Menu_Print(-1);
    }

	while ( 1 )
	{
        BackgroundProcess(false);

        switch ( KEY_Get() )
        {
#ifdef __AUTOMATIC_DOOR__
            case KEY_OPEN:
#ifdef __SB_VER_2__
                if ( DoorIsOpened() ) DoorClose();
                else DoorOpen();
#else
                DoorOpen();
#endif
                break;

            case KEY_CLOSE:
                DoorClose();
                break;
#endif
            case KEY_UP:
                if ( Menu.input_active )
                    Input_ChangeValue(1);
                else
                    Menu_Move(-1);
                break;
            case KEY_DOWN:
                if ( Menu.input_active )
                    Input_ChangeValue(-1);
                else
                    Menu_Move(1);
                break;
            case KEY_LEFT:
                if ( Menu.input_active )
                    Input_MoveByCursor(-1);
                break;
            case KEY_RIGHT:
                if ( Menu.input_active )
                    Input_MoveByCursor(1);
                break;
            case KEY_ENTER:
                Menu_InputSwitch();
                switch ( Menu.focus_mid )
                {
                    case 0:
                        RB_PowerOnAndCheck(0);
                        while ( gRotor.status != ROTOR_READY ) BackgroundProcess(true);

                        if ( gRotor.result != SUCCESS )
                        {
                            RotorPower(0);
                            SlipRingPower(0);
                            Buzzer(2500, 200);
                            break;
                        }

                        RB_GetInformation();
                        while ( gRotor.status != ROTOR_FINISHED ) BackgroundProcess(true);

                        RotorPower(0);
                        SlipRingPower(0);

                        if ( gRotor.result != SUCCESS )
                        {
                            Buzzer(2500, 200);
                        }
                        else
                        {
                            Input_SetValueDigit(Menu.item[3].field_id, Int2Str(NULL, gRotor.default_pos[0], 4, true), true);
                            Input_SetValueDigit(Menu.item[4].field_id, Int2Str(NULL, gRotor.default_pos[1], 4, true), true);
                            Input_SetValueDigit(Menu.item[5].field_id, Int2Str(NULL, gRotor.default_pos[2], 4, true), true);

                            Buzzer(1000, 30);
                            Buzzer(1500, 30);
                            Buzzer(2000, 30);
                        }
                        break;
                    case 1:
                        goto save_process;
                }
                break;

            case KEY_SAVE:
save_process:
                Input_GetValue(Menu.item[3].field_id, &gRotor.default_pos[0], NULL);
                Input_GetValue(Menu.item[4].field_id, &gRotor.default_pos[1], NULL);
                Input_GetValue(Menu.item[5].field_id, &gRotor.default_pos[2], NULL);

                RB_PowerOnAndCheck(0);
                while ( gRotor.status != ROTOR_READY ) BackgroundProcess(true);

                if ( gRotor.result != SUCCESS )
                {
                    RotorPower(0);
                    SlipRingPower(0);
                    Buzzer(2500, 200);
                    break;;
                }

                RB_SetWeightDefaultPosition(gRotor.default_pos[0], gRotor.default_pos[1], gRotor.default_pos[2]);
                while ( gRotor.status != ROTOR_FINISHED ) BackgroundProcess(true);

                RotorPower(0);
                SlipRingPower(0);

                if ( gRotor.result != SUCCESS )
                {
                    Buzzer(2500, 200);
                }
                else
                {
                    Buzzer(1000, 30);
                    Buzzer(1500, 30);
                    Buzzer(2000, 30);
                }
                break;

            case KEY_STOP:
                return;
        }
    }
}

void EditOptions(void)
{
    int value;
    const static char* option_str_onoff[2] = { "NO", "YES" };

    //FRAM_Read(100, &swing_rotor_param, sizeof(swing_rotor_param));

    LCD_ClearScreen();
    FillLine2(0, 0, 1);
    LCD_PRINT(3, 0, "Edit Options", 0);

    // 메뉴 초기화
    {
        Menu_Init(1);
        Menu_Add_InputOption("DEBUG: ", 11, (char**)option_str_onoff, gDebugMode, 0, 1, 3);
        Menu_Add_InputOption("LOCK #19: ", 11, (char**)option_str_onoff, gSetting.lock_program_19, 0, 1, 3);
        Menu_Add_InputInteger("TEMP CAL:", 11, "%i", gSetting.temp_calibration_value, -10, 10, 1);
        Menu_Print(-1);
    }

	while ( 1 )
	{
        BackgroundProcess(false);

        switch ( KEY_Get() )
        {
#ifdef __AUTOMATIC_DOOR__
            case KEY_OPEN:
#ifdef __SB_VER_2__
                if ( DoorIsOpened() ) DoorClose();
                else DoorOpen();
#else
                DoorOpen();
#endif
                break;

            case KEY_CLOSE:
                DoorClose();
                break;
#endif
            case KEY_UP:
                if ( Menu.input_active )
                    Input_ChangeValue(1);
                else
                    Menu_Move(-1);
                break;
            case KEY_DOWN:
                if ( Menu.input_active )
                    Input_ChangeValue(-1);
                else
                    Menu_Move(1);
                break;
            case KEY_LEFT:
                if ( Menu.input_active )
                    Input_MoveByCursor(-1);
                break;
            case KEY_RIGHT:
                if ( Menu.input_active )
                    Input_MoveByCursor(1);
                break;
            case KEY_ENTER:
                Menu_InputSwitch();
                break;
            case KEY_SAVE:
                {
                    Input_GetValue(Menu.item[0].field_id, &value, NULL);
                    gDebugMode = gSetting.debug = value;
                    Input_GetValue(Menu.item[1].field_id, &value, NULL);
                    gSetting.lock_program_19 = value;
                    Input_GetValue(Menu.item[2].field_id, &value, NULL);
                    gSetting.temp_calibration_value = value;

                    FRAM_Write(800, &gSetting, sizeof(gSetting));
                }
            case KEY_STOP:
                return;
        }
    }
}
