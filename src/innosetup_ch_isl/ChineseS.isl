; *** Inno Setup version 5.1.0+ English messages ***
;
; To download user-contributed translations of this file, go to:
;   http://www.jrsoftware.org/is3rdparty.php
;
; Note: When translating this text, do not add periods (.) to the end of
; messages that didn't have them already, because on those messages Inno
; Setup adds the periods automatically (appending a period would result in
; two periods being displayed).
[LangOptions]
; The following three entries are very important. Be sure to read and
; understand the '[LangOptions] section' topic in the help file.
LanguageName=Simplified-Chinese
LanguageID=$0004
LanguageCodePage=0
; If the language you are translating to requires special font faces or
; sizes, uncomment any of the following entries and change them accordingly.
;DialogFontName=
;DialogFontSize=9
;WelcomeFontName= 宋体
;WelcomeFontSize=12
;TitleFontName=宋体
;TitleFontSize=29
;CopyrightFontName= 宋体
;CopyrightFontSize=9
[Messages]
; *** Application titles
SetupAppTitle=安装
SetupWindowTitle=安装 - %1
UninstallAppTitle=卸载
UninstallAppFullTitle=%1 卸载
; *** Misc. common
InformationTitle=信息
ConfirmTitle=确认
ErrorTitle= 错误
; *** SetupLdr messages
SetupLdrStartupMessage=将要安装 %1。您希望继续吗？
LdrCannotCreateTemp= 无法创建临时文件。安装程序被终止
LdrCannotExecTemp=无法执行临时目录下的文件。安装程序被终止
; *** Startup error messages
LastErrorMessage=%1.%n%n错误 %2: %3
SetupFileMissing= 安装目录下的 %1 文件已丢失。请检查问题或重新获得一程序复本。
SetupFileCorrupt=安装文件已损坏。请重新获得一程序复本。
SetupFileCorruptOrWrongVer= 安装文件已损坏，或者是与当前版本的安装程序不兼容。请检查问题或重新获得一程序复本。
NotOnThisPlatform=程序不能在 %1 上运行。
OnlyOnThisPlatform=程序必须在 %1 上运行。
OnlyOnTheseArchitectures=该程序只能在下列处理器架构的Windows版本中安装：%n%n%1
MissingWOW64APIs=您正在运行的Windows版本不具备执行 64位安装的功能。要解决此问题，请安装补丁 %1。
WinVersionTooLowError=本程序需要 %1 %2 版本或更高版本。
WinVersionTooHighError= 本程序不能在 %1 %2 版本或更高版本下安装。
AdminPrivilegesRequired=您必须先以管理员身份登录才能安装该程序。
PowerUserPrivilegesRequired= 您必须以管理员身份或Power Users Group成员登录才能安装该程序。
SetupAppRunningError=安装程序检查到有一个 %1 正在运行。%n%n请现在关闭它的所有应用，然后单击确定继续，或取消退出。
UninstallAppRunningError=卸载程序检查到有一个 %1 正在运行。%n%n请现在关闭它的所有应用，然后单击确定继续，或取消退出。
; *** Misc. errors
ErrorCreatingDir=安装程序无法创建目录"%1"
ErrorTooManyFilesInDir= 无法在"%1"目录下创建文件，因为它包含了太多文件
; *** Setup common messages
ExitSetupTitle=结束安装
ExitSetupMessage= 安装尚未完成。如果您现在退出，程序将不能被安装。%n%n您可以另外选择时间完成安装。%n%n要结束安装吗？
AboutSetupMenuItem= 关于安装(&A)...
AboutSetupTitle=关于安装
AboutSetupMessage=%1 版本 %2%n%3%n%n%1 主页:%n%4
AboutSetupNote=
TranslatorNote=
; *** Buttons
ButtonBack=< 上一步(&B)
ButtonNext=下一步(&N) >
ButtonInstall=安装(&I)
ButtonOK=确定
ButtonCancel=取消
ButtonYes=是(&Y)
ButtonYesToAll=全部都是(&A)
ButtonNo=否(&N)
ButtonNoToAll=全部都否(&o)
ButtonFinish=完成(&F)
ButtonBrowse= 浏览(&B)...
ButtonWizardBrowse=浏览(&R)...
ButtonNewFolder=创建新文件夹(&M)
; *** "Select Language" dialog messages
SelectLanguageTitle=选择安装语言
SelectLanguageLabel=选择安装期间所使用的语言:
; *** Common wizard text
ClickNext=单击『下一步』继续，退出安装请按『取消』
BeveledLabel=
BrowseDialogTitle= 浏览文件夹
BrowseDialogLabel=在下面的列表中选择一个文件夹, 并单击『确定』
NewFolderName=新建文件夹
; *** "Welcome" wizard page
WelcomeLabel1=欢迎使用[name]安装向导
WelcomeLabel2=[name/ver] 即将安装到您的计算机中。%n%n建议您在继续安装之前关闭所有其它应用程序。
; *** "Password" wizard page
WizardPassword=密码
PasswordLabel1= 本次安装受密码保护
PasswordLabel3=请输入密码并单击『下一步』继续。密码为大小写敏感字符。
PasswordEditLabel= 密码(&P)：
IncorrectPassword=您输入的密码不正确, 请重新输入。
; *** "License Agreement" wizard page
WizardLicense=许可协议确认
LicenseLabel= 请在继续后面操作前阅读如下重要信息.
LicenseLabel3=请阅读后面的许可协议。您在继续安装之前必须接受本协议中的条款。
LicenseAccepted= 我愿意接受本协议条款(&A)
LicenseNotAccepted=我拒绝接受本协议条款(&D)
; *** "Information" wizard pages
WizardInfoBefore=信息
InfoBeforeLabel= 请在继续后面操作前阅读如下重要信息.
InfoBeforeClickLabel=但您准备好继续安装时，请单击『下一步』。
WizardInfoAfter= 信息
InfoAfterLabel=在继续之前，请阅读后面重要信息。
InfoAfterClickLabel=但您准备好继续安装时，请单击『下一步』。
; *** "User Information" wizard page
WizardUserInfo=用户信息
UserInfoDesc= 请输入您的信息.
UserInfoName=用户姓名(&U):
UserInfoOrg=组织(&O):
UserInfoSerial= 序列号(&S):
UserInfoNameRequired=您必须输入一个名字。
; *** "Select Destination Location" wizard page
WizardSelectDir= 选择目标位置
SelectDirDesc=[name]要被安装到哪里？
SelectDirLabel3=[name]将被安装到后面这个文件夹中。
SelectDirBrowseLabel=继续请单击『下一步』。如果您要另选一个文件夹，请单击『浏览』。
DiskSpaceMBLabel= 剩余磁盘空间至少要有 [mb] MB。
ToUNCPathname=无法安装到 UNC 路径。如果您要尝试安装到网络中，请先映射一个网络驱动器。
InvalidPath=您必须输入一个带盘符的全路径名，例如：%n%nC:\APP%n%n或者一个 UNC 路径：%n%n\\server\share
InvalidDrive=您选择的驱动盘或 UNC共享不存在或不可访问。请选择其它驱动盘或UNC共享。
DiskSpaceWarningTitle=磁盘空间不足
DiskSpaceWarning= 安装程序至少需要 %1 KB 的磁盘空间，但所选中的驱动盘仅有 %2 KB 可用。%n%n您仍然要继续吗？
DirNameTooLong= 文件夹名称或路径太长。
InvalidDirName=文件夹名称不合法。
BadDirName32=文件夹名称不能含有以下字符：%n%n%1
DirExistsTitle=文件夹存在
DirExists=文件夹：%n%n%1%n%n已经存在。您仍然要安装到此文件夹中吗？
DirDoesntExistTitle=文件夹不存在
DirDoesntExist=文件夹：%n%n%1%n%n不存在。您要创建它吗？
; *** "Select Components" wizard page
WizardSelectComponents=选择组件
SelectComponentsDesc=哪些组件会被安装？
SelectComponentsLabel2=选中您要安装的组件；清除您不想安装的组件。当您准备继续时单击『下一步』。
FullInstallation=完全安装
; if possible don't translate 'Compact' as 'Minimal' (I mean 'Minimal' in your language)
CompactInstallation=精简安装
CustomInstallation=自定义安装
NoUninstallWarningTitle= 组件存在
NoUninstallWarning=安装程序检测到您的计算机中已经安装了如下组件：%n%n%1%n%n取消这些组件将不会安装它们。%n%n您现在要继续吗？
ComponentSize1=%1 KB
ComponentSize2=%1 MB
ComponentsDiskSpaceMBLabel= 目前选中的至少需要 [mb] MB 的磁盘空间。
; *** "Select Additional Tasks" wizard page
WizardSelectTasks=选择额外任务
SelectTasksDesc=还有哪些任务会被执行呢？
SelectTasksLabel2=选择您想在安装[name]的过程中执行的其它额外任务，然后单击『下一步』。
; *** "Select Start Menu Folder" wizard page
WizardSelectProgramGroup= 选择开始菜单
SelectStartMenuFolderDesc=安装程序会在哪儿创建程序的快捷方式？
SelectStartMenuFolderLabel3= 安装程序将在如下的开始菜单文件夹中创建程序的快捷方式。
SelectStartMenuFolderBrowseLabel=单击『下一步』继续。如果您要选择其它文件夹，请单击『浏览』。
MustEnterGroupName=您必须输入一个文件夹名称。
GroupNameTooLong= 文件夹名称或路径太长了。
InvalidGroupName=文件夹名称不合法。
BadGroupName=文件夹名称不能含有如下字符：%n%n%1
NoProgramGroupCheck2=不创建开始菜单文件夹(&D)
; *** "Ready to Install" wizard page
WizardReady=准备安装
ReadyLabel1= 安装程序正准备在您的机器中安装[name].
ReadyLabel2a=继续安装请单击『安装』，如果您要回顾或修改一些设置，请单击『上一步』。
ReadyLabel2b=继续安装请单击『安装』。
ReadyMemoUserInfo=用户信息:
ReadyMemoDir= 目标位置:
ReadyMemoType=安装类型:
ReadyMemoComponents=选中的组件:
ReadyMemoGroup= 开始菜单夹:
ReadyMemoTasks=附加任务项:
; *** "Preparing to Install" wizard page
WizardPreparing=准备安装
PreparingDesc= 安装程序正准备在您的机器中安装[name].
PreviousInstallNotCompleted=之前有一个安装/卸载程序尚未完成。您得重新启动计算机完成该安装。%n%n重启机器之后，请再次运行安装程序以完成[name]的安装。
CannotContinue=安装程序无法继续。请单击『取消』退出。
; *** "Installing" wizard page
WizardInstalling=安装中
InstallingLabel= 正在安装[name], 请等待.
; *** "Setup Completed" wizard page
FinishedHeadingLabel=完成 [name]安装向导
FinishedLabelNoIcons=安装程序已完成[name]的安装.
FinishedLabel=安装程序已经在您的计算机中安装了[name]。选中安装后的图标可启动应用程序。
ClickFinish=单击『完成』退出安装。
FinishedRestartLabel= 为能完成[name]安装过程, 现在需要重启机器, 您愿意吗?
FinishedRestartMessage=要完成[name]的安装，必须重新启动您的计算机。%n%n您愿意现在重启吗？
ShowReadmeCheck=是的, 我要阅读README文件
YesRadio= (&Y)是的, 现在重启计算机
NoRadio=(&N)不, 我以后再重启计算机
; used for example as 'Run MyProg.exe'
RunEntryExec=运行 %1
; used for example as 'View Readme.txt'
RunEntryShellExec=阅读 %1
; *** "Setup Needs the Next Disk" stuff
ChangeDiskTitle=安装程序需要下一个磁盘
SelectDiskLabel2=请插入磁盘 %1 并单击确定。%n%n如果除了以下列出的之外，仍未找到磁盘文件的话，请输入正确路径或单击『浏览』。
PathLabel=路径(& P)：
FileNotInDir2="%2" 中无法定位文件 "%1"。请插入正确磁盘或选择其它文件夹。
SelectDirectoryLabel= 请指定下一张磁盘的位置。
; *** Installation phase messages
SetupAborted=安装未完成。%n%n请检查问题并重新运行安装程序。
EntryAbortRetryIgnore=重试请单击『重试』，忽略将会继续进行，终止将取消安装。
; *** Installation status messages
StatusCreateDirs=正在创建文件路径...
StatusExtractFiles= 正在释放文件...
StatusCreateIcons=正在创建快捷方式...
StatusCreateIniEntries=正在创建INI入口...
StatusCreateRegistryEntries=正在创建注册表入口...
StatusRegisterFiles= 正在注册文件...
StatusSavingUninstall=正在保存反安装信息...
StatusRunProgram=正在结束安装...
StatusRollback=正在恢复到原先的状态...
; *** Misc. errors
ErrorInternal2=内部错误: %1
ErrorFunctionFailedNoCode=%1 失败
ErrorFunctionFailed=%1 失败；代码为 %2
ErrorFunctionFailedWithMessage=%1 失败；代码为 %2.%n%3
ErrorExecutingProgram=无法执行文件：%n%1
; *** Registry errors
ErrorRegOpenKey=打开注册表键值错：%n%1\%2
ErrorRegCreateKey= 创建注册表键值错：%n%1\%2
ErrorRegWriteKey=写入注册表键值错：%n%1\%2
; *** INI errors
ErrorIniEntry=在文件 "%1" 中创建INI入口信息错。
; *** File copying errors
FileAbortRetryIgnore=单击『重试』可重新尝试一次，忽略可跳过该文件（不推荐），或终止退出安装。
FileAbortRetryIgnore2=单击『重试』可重新尝试一次，忽略可继续执行（不推荐），或终止退出安装。
SourceIsCorrupted=源文件已损坏
SourceDoesntExist=源文件 "%1" 不存在
ExistingFileReadOnly= 已存在的文件为只读文件。%n%n单击重试可移除只读属性，然后重试。忽略可跳过此文件，终止可取消安装。
ErrorReadingExistingDest= 尝试读文件时有错误发生：
FileExists=文件已存在。%n%n想要安装程序覆盖它吗？
ExistingFileNewer=已存在的文件版本较新，建议保留该文件。%n%n您愿意保留该文件吗？
ErrorChangingAttr=尝试修改文件属性时有错误发生：
ErrorCreatingTemp= 尝试在目标路径下创建文件时有错误发生：
ErrorReadingSource=尝试读源文件时有错误发生：
ErrorCopying= 尝试复制文件时有错误发生：
ErrorReplacingExistingFile=尝试替换文件时有错误发生：
ErrorRestartReplace=RestartReplace failed:
ErrorRenamingTemp=尝试重命名目标路径的文件时有错误发生：
ErrorRegisterServer= 无法注册 DLL/OCX：%1
ErrorRegSvr32Failed=RegSvr32 失败退出码 %1
;ErrorRegisterServerMissingExport=未发现DllRegisterServer 的导出接口
ErrorRegisterTypeLib=无法注册类型库：%1
; *** Post-installation errors
ErrorOpeningReadme=尝试打开 README 文件时有错误发生。
ErrorRestartingComputer=安装程序无法重启计算机，请手动操作。
; *** Uninstaller messages
UninstallNotFound="%1"文件不存在。无法卸载。
UninstallOpenError="%1" 文件无法打开。无法卸载
UninstallUnsupportedVer=该版本的卸载工具无法识别当前卸载日志"%1"中的格式。无法卸载
UninstallUnknownEntry= 在卸载程序的日志中遇到不可识别的入口信息(%1)
ConfirmUninstall=您确定要完全删除%1以及它的相关组件吗?
UninstallOnlyOnWin64= 该安装程序只能在64位Windows操作系统下被卸载。
OnlyAdminCanUninstall=该安装程序只能由具有管理员权限的用户卸载。
UninstallStatusLabel=%1 将从您的计算机中被卸载，请等待。
UninstalledAll=%1 已成功被卸载。
UninstalledMost=%1 卸载完成。%n%n但仍有些文件未被移除，请您手动删除它们。
UninstalledAndNeedsRestart= 要完成本次%1的卸载，您必须重启机器。%n%n您愿意现在重启吗？
UninstallDataCorrupted="%1" 文件已损坏，无法卸载
; *** Uninstallation phase messages
ConfirmDeleteSharedFileTitle= 要删除共享文件吗?
ConfirmDeleteSharedFile2=以下共享文件不再被任何程序使用，您要卸载并删除这些共享文件吗？%n%n一旦它被删除，若有其它程序仍要使用该文件的话，那些程序的功能将会不完整。如果您不确定，请选择『否』。保留该文件在系统中不会引起任何损害。
SharedFileNameLabel=文件名称:
SharedFileLocationLabel=位置:
WizardUninstalling= 卸载状态
StatusUninstalling=正在卸载 %1...
; The custom messages below aren't used by Setup itself, but if you make
; use of them in your scripts, you'll want to translate them.
[CustomMessages]
NameAndVersion=%1 版本 %2
AdditionalIcons=其它图标：
CreateDesktopIcon= 创建桌面图标(&d)
CreateQuickLaunchIcon=创建快速启动图标(&Q)
ProgramOnTheWeb=%1 相关网站
UninstallProgram=卸载 %1
LaunchProgram=运行 %1
AssocFileExtension=%1 将与文件扩展名%2建立关联(&A)
AssocingFileExtension=正在建立%1与文件扩展名%2的关联关系...