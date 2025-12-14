; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CResConvertDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "ResConvert.h"

ClassCount=3
Class1=CResConvertApp
Class2=CResConvertDlg
Class3=CAboutDlg

ResourceCount=3
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDD_RESCONVERT_DIALOG

[CLS:CResConvertApp]
Type=0
HeaderFile=ResConvert.h
ImplementationFile=ResConvert.cpp
Filter=N

[CLS:CResConvertDlg]
Type=0
HeaderFile=ResConvertDlg.h
ImplementationFile=ResConvertDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=IDC_PROGRESS_STATUS

[CLS:CAboutDlg]
Type=0
HeaderFile=ResConvertDlg.h
ImplementationFile=ResConvertDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_RESCONVERT_DIALOG]
Type=1
Class=CResConvertDlg
ControlCount=14
Control1=IDC_STATIC,static,1342308865
Control2=IDC_RESFILEPATH,edit,1350633600
Control3=IDC_SELECTFILE,button,1342242816
Control4=IDC_STATIC,button,1342177287
Control5=IDC_RESTREE,SysTreeView32,1342242823
Control6=IDC_RESCHANGE,button,1342242817
Control7=IDC_STATIC,button,1342177287
Control8=IDC_SELECT,button,1342242816
Control9=IDC_STYLELIST,SysListView32,1350631427
Control10=IDC_REFRESH,button,1342242816
Control11=IDC_PROGRESS_STATUS,msctls_progress32,1342177280
Control12=IDC_EDIT_INFO,edit,1345390596
Control13=IDC_BTN_CLEAN,button,1342242816
Control14=IDC_CHECK_RUNTAG,button,1342242819

