# Microsoft Developer Studio Project File - Name="TMCast_Static" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=TMCast_Static - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE run the tool that generated this project file and specify the
!MESSAGE nmake output type.  You can then use the following command:
!MESSAGE
!MESSAGE NMAKE /f "TMCast_Static.mak".
!MESSAGE
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "TMCast_Static.mak" CFG="TMCast_Static - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "TMCast_Static - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "TMCast_Static - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TMCast_Static - Win32 Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Static_Release"
# PROP Intermediate_Dir "Static_Release\TMCast_Static"
# PROP Target_Dir ""
LINK32=link.exe -lib
# ADD CPP /nologo /O2 /W3 /GX /MD /GR /I "..\.." /D NDEBUG /D WIN32 /D _WINDOWS /D ACE_AS_STATIC_LIBS /FD /c
# SUBTRACT CPP /YX

# ADD MTL /D "NDEBUG" /nologo /mktyplib203 /win32
# ADD RSC /l 0x409 /d NDEBUG /i "..\.."
BSC32=bscmake.exe
# ADD BSC32 /nologo /o"..\..\lib\TMCast.bsc"
LIB32=link.exe -lib
# ADD LIB32 /nologo /out:"..\..\lib\TMCasts.lib"

!ELSEIF  "$(CFG)" == "TMCast_Static - Win32 Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Static_Debug"
# PROP Intermediate_Dir "Static_Debug\TMCast_Static"
# PROP Target_Dir ""
LINK32=link.exe -lib
# ADD CPP /nologo /Ob0 /W3 /Gm /GX /Zi /MDd /GR /Gy /I "..\.." /D _DEBUG /D WIN32 /D _WINDOWS /D ACE_AS_STATIC_LIBS /FD /c
# SUBTRACT CPP /Fr /YX

# ADD MTL /D "_DEBUG" /nologo /mktyplib203 /win32
# ADD RSC /l 0x409 /d _DEBUG /i "..\.."
BSC32=bscmake.exe
# ADD BSC32 /nologo /o"..\..\lib\TMCast.bsc"
LIB32=link.exe -lib
# ADD LIB32 /nologo /out:"..\..\lib\TMCastsd.lib"

!ENDIF

# Begin Target

# Name "TMCast_Static - Win32 Release"
# Name "TMCast_Static - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;cxx;c"
# Begin Source File

SOURCE="Group.cpp"
# End Source File
# Begin Source File

SOURCE="MTQueue.cpp"
# End Source File
# Begin Source File

SOURCE="Protocol.cpp"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hh"
# Begin Source File

SOURCE="Export.hpp"
# End Source File
# Begin Source File

SOURCE="FaultDetector.hpp"
# End Source File
# Begin Source File

SOURCE="Group.hpp"
# End Source File
# Begin Source File

SOURCE="GroupFwd.hpp"
# End Source File
# Begin Source File

SOURCE="LinkListener.hpp"
# End Source File
# Begin Source File

SOURCE="Messaging.hpp"
# End Source File
# Begin Source File

SOURCE="MTQueue.hpp"
# End Source File
# Begin Source File

SOURCE="Protocol.hpp"
# End Source File
# Begin Source File

SOURCE="TransactionController.hpp"
# End Source File
# End Group
# Begin Group "Documentation"

# PROP Default_Filter ""
# Begin Source File

SOURCE="README"
# End Source File
# End Group
# End Target
# End Project
