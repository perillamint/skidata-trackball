/*
  Copyright 2018  Dean Camera (dean [at] fourwalledcubicle [dot] com)
  Copyright 2018  perillamint

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#include "skidata.h"

/** Buffer to hold the previously generated Mouse HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevMouseHIDReportBuffer[sizeof(USB_MouseReport_Data_t)];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another. This is for the mouse HID
 *  interface within the device.
 */
USB_ClassInfo_HID_Device_t Mouse_HID_Interface =
    {
        .Config =
            {
                .InterfaceNumber              = INTERFACE_ID_Mouse,
                .ReportINEndpoint             =
                    {
                        .Address              = MOUSE_IN_EPADDR,
                        .Size                 = HID_EPSIZE,
                        .Banks                = 1,
                    },
                .PrevReportINBuffer           = PrevMouseHIDReportBuffer,
                .PrevReportINBufferSize       = sizeof(PrevMouseHIDReportBuffer),
            },
    };

uint8_t enc_prev[2] = {0, 0};
uint8_t enc_cur[2] = {0, 0};

int16_t xpos = 0;
int16_t ypos = 0;

uint8_t buttonstat = 0x00;

void handle_xenc_int() {
    enc_cur[0] = (PIND & 0x0C) >> 2;
    xpos += dirtbl[enc_prev[0] << 2 | enc_cur[0]];
    enc_prev[0] = enc_cur[0];
}

void handle_yenc_int() {
    enc_cur[1] = (PIND & 0x03) >> 0;
    ypos += dirtbl[enc_prev[1] << 2 | enc_cur[1]];
    enc_prev[1] = enc_cur[1];
}

ISR(INT0_vect) {
    handle_yenc_int();
}

ISR(INT1_vect) {
    handle_yenc_int();
}

ISR(INT2_vect) {
    handle_xenc_int();
}

ISR(INT3_vect) {
    handle_xenc_int();
}

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
    SetupHardware();

    GlobalInterruptEnable();

    for (;;)
        {
            HID_Device_USBTask(&Mouse_HID_Interface);
            USB_USBTask();
        }
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware()
{
    /* Disable watchdog if enabled by bootloader/fuses */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    /* Disable clock division */
    clock_prescale_set(clock_div_1);

    /* Hardware Initialization */
    USB_Init();

    DDRD &= 0xF0;
    PORTD &= 0x00;

    DDRB = 0xE1;
    PORTB = 0xE0;

    DDRF = 0x00;
    PORTF = 0x00;

    EICRA = 0x55;
    EIMSK = 0x0F;

    //PCICR = 0x01;
    //PCMSK0 = 0x1E;

    // Check any button is pressed or not

    //uint8_t tmp = 0;
    //for(;;) {
    //uint8_t buttonstat = (PINF);

    //if (buttonstat != tmp) {
    //    PORTB ^= 0x01;
    //    tmp = buttonstat;
    //}
    //}
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
    // Do nothing
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
    // Do nothing
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
    bool ConfigSuccess = true;

    ConfigSuccess &= HID_Device_ConfigureEndpoints(&Mouse_HID_Interface);

    USB_Device_EnableSOFEvents();
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
    HID_Device_ProcessControlRequest(&Mouse_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
    HID_Device_MillisecondElapsed(&Mouse_HID_Interface);
}

/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{

    USB_MouseReport_Data_t* MouseReport = (USB_MouseReport_Data_t*)ReportData;

    MouseReport->Y = -ypos;
    MouseReport->X = xpos;
    MouseReport->Button = ((~PINF & 0xF0) >> 4);

    PORTB = (~PINF & 0xF0) >> 4;

    xpos = 0;
    ypos = 0;

    /* If first board button being held down, no mouse report */
//    if (ButtonStatus_LCL & BUTTONS_BUTTON1)
//      return 0;

//    if (JoyStatus_LCL & JOY_UP)
//      MouseReport->Y = -1;
//    else if (JoyStatus_LCL & JOY_DOWN)
//      MouseReport->Y =  1;
//
//    if (JoyStatus_LCL & JOY_LEFT)
//      MouseReport->X = -1;
//    else if (JoyStatus_LCL & JOY_RIGHT)
//      MouseReport->X =  1;
//
//    if (JoyStatus_LCL & JOY_PRESS)
//      MouseReport->Button |= (1 << 0);

    *ReportSize = sizeof(USB_MouseReport_Data_t);
    return true;
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
    // Do nothing
}
