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
LanguageName=Traditional-Chinese
LanguageID=$7C04
LanguageCodePage=0
; If the language you are translating to requires special font faces or
; sizes, uncomment any of the following entries and change them accordingly.
;DialogFontName=
;DialogFontSize=9
;WelcomeFontName= 宋體
;WelcomeFontSize=12
;TitleFontName=宋體
;TitleFontSize=29
;CopyrightFontName= 宋體
;CopyrightFontSize=9
[Messages]
; *** Application titles
SetupAppTitle=安裝
SetupWindowTitle=安裝 - %1
UninstallAppTitle=卸載
UninstallAppFullTitle=%1 卸載
; *** Misc. common
InformationTitle=信息
ConfirmTitle=確認
ErrorTitle= 錯誤
; *** SetupLdr messages
SetupLdrStartupMessage=將要安裝 %1。您希望繼續嗎？
LdrCannotCreateTemp= 無法創建暫存檔案。安裝程式被終止
LdrCannotExecTemp=無法執行臨時目錄下的檔。安裝程式被終止
; *** Startup error messages
LastErrorMessage=%1.%n%n錯誤 %2: %3
SetupFileMissing= 安裝目錄下的 %1 檔已丟失。請檢查問題或重新獲得一程式複本。
SetupFileCorrupt=安裝檔已損壞。請重新獲得一程式複本。
SetupFileCorruptOrWrongVer= 安裝檔已損壞，或者是與當前版本的安裝程式不相容。請檢查問題或重新獲得一程式複本。
NotOnThisPlatform=程式不能在 %1 上運行。
OnlyOnThisPlatform=程式必須在 %1 上運行。
OnlyOnTheseArchitectures=該程式只能在下列處理器架構的Windows版本中安裝：%n%n%1
MissingWOW64APIs=您正在運行的Windows版本不具備執行 64位安裝的功能。要解決此問題，請安裝補丁 %1。
WinVersionTooLowError=本程式需要 %1 %2 版本或更高版本。
WinVersionTooHighError= 本程式不能在 %1 %2 版本或更高版本下安裝。
AdminPrivilegesRequired=您必須先以管理員身份登錄才能安裝該程式。
PowerUserPrivilegesRequired= 您必須以管理員身份或Power Users Group成員登錄才能安裝該程式。
SetupAppRunningError=安裝程式檢查到有一個 %1 正在運行。%n%n請現在關閉它的所有應用，然後單擊確定繼續，或取消退出。
UninstallAppRunningError=卸載程式檢查到有一個 %1 正在運行。%n%n請現在關閉它的所有應用，然後單擊確定繼續，或取消退出。
; *** Misc. errors
ErrorCreatingDir=安裝程式無法創建目錄"%1"
ErrorTooManyFilesInDir= 無法在"%1"目錄下創建檔，因為它包含了太多檔
; *** Setup common messages
ExitSetupTitle=結束安裝
ExitSetupMessage= 安裝尚未完成。如果您現在退出，程式將不能被安裝。%n%n您可以另外選擇時間完成安裝。%n%n要結束安裝嗎？
AboutSetupMenuItem= 關於安裝(&A)...
AboutSetupTitle=關於安裝
AboutSetupMessage=%1 版本 %2%n%3%n%n%1 主頁:%n%4
AboutSetupNote=
TranslatorNote=
; *** Buttons
ButtonBack=< 上一步(&B)
ButtonNext=下一步(&N) >
ButtonInstall=安裝(&I)
ButtonOK=確定
ButtonCancel=取消
ButtonYes=是(&Y)
ButtonYesToAll=全部都是(&A)
ButtonNo=否(&N)
ButtonNoToAll=全部都否(&o)
ButtonFinish=完成(&F)
ButtonBrowse= 流覽(&B)...
ButtonWizardBrowse=流覽(&R)...
ButtonNewFolder=創建新文件夾(&M)
; *** "Select Language" dialog messages
SelectLanguageTitle=選擇安裝語言
SelectLanguageLabel=選擇安裝期間所使用的語言:
; *** Common wizard text
ClickNext=單擊『下一步』繼續，退出安裝請按『取消』
BeveledLabel=
BrowseDialogTitle= 流覽檔夾
BrowseDialogLabel=在下面的列表中選擇一個檔夾, 並單擊『確定』
NewFolderName=新建文件夾
; *** "Welcome" wizard page
WelcomeLabel1=歡迎使用[name]安裝嚮導
WelcomeLabel2=[name/ver] 即將安裝到您的電腦中。%n%n建議您在繼續安裝之前關閉所有其他應用程式。
; *** "Password" wizard page
WizardPassword=密碼
PasswordLabel1= 本次安裝受密碼保護
PasswordLabel3=請輸入密碼並單擊『下一步』繼續。密碼為大小寫敏感字元。
PasswordEditLabel= 密碼(&P)：
IncorrectPassword=您輸入的密碼不正確, 請重新輸入。
; *** "License Agreement" wizard page
WizardLicense=許可協議確認
LicenseLabel= 請在繼續後面操作前閱讀如下重要資訊.
LicenseLabel3=請閱讀後面的許可協議。您在繼續安裝之前必須接受本協議中的條款。
LicenseAccepted= 我願意接受本協議條款(&A)
LicenseNotAccepted=我拒絕接受本協議條款(&D)
; *** "Information" wizard pages
WizardInfoBefore=信息
InfoBeforeLabel= 請在繼續後面操作前閱讀如下重要資訊.
InfoBeforeClickLabel=但您準備好繼續安裝時，請單擊『下一步』。
WizardInfoAfter= 信息
InfoAfterLabel=在繼續之前，請閱讀後面重要資訊。
InfoAfterClickLabel=但您準備好繼續安裝時，請單擊『下一步』。
; *** "User Information" wizard page
WizardUserInfo=用戶資訊
UserInfoDesc= 請輸入您的資訊.
UserInfoName=用戶姓名(&U):
UserInfoOrg=組織(&O):
UserInfoSerial= 序列號(&S):
UserInfoNameRequired=您必須輸入一個名字。
; *** "Select Destination Location" wizard page
WizardSelectDir= 選擇目標位置
SelectDirDesc=[name]要被安裝到哪里？
SelectDirLabel3=[name]將被安裝到後面這個檔夾中。
SelectDirBrowseLabel=繼續請單擊『下一步』。如果您要另選一個檔夾，請單擊『流覽』。
DiskSpaceMBLabel= 剩餘磁碟空間至少要有 [mb] MB。
ToUNCPathname=無法安裝到 UNC 路徑。如果您要嘗試安裝到網路中，請先映射一個網路驅動器。
InvalidPath=您必須輸入一個帶盤符的全路徑名，例如：%n%nC:\APP%n%n或者一個 UNC 路徑：%n%n\\server\share
InvalidDrive=您選擇的驅動盤或 UNC共用不存在或不可訪問。請選擇其他驅動盤或UNC共用。
DiskSpaceWarningTitle=磁碟空間不足
DiskSpaceWarning= 安裝程式至少需要 %1 KB 的磁碟空間，但所選中的驅動盤僅有 %2 KB 可用。%n%n您仍然要繼續嗎？
DirNameTooLong= 檔夾名稱或路徑太長。
InvalidDirName=檔夾名稱不合法。
BadDirName32=檔夾名稱不能含有以下字元：%n%n%1
DirExistsTitle=文件夾存在
DirExists=文件夾：%n%n%1%n%n已經存在。您仍然要安裝到此檔夾中嗎？
DirDoesntExistTitle=文件夾不存在
DirDoesntExist=文件夾：%n%n%1%n%n不存在。您要創建它嗎？
; *** "Select Components" wizard page
WizardSelectComponents=選擇組件
SelectComponentsDesc=哪些組件會被安裝？
SelectComponentsLabel2=選中您要安裝的組件；清除您不想安裝的組件。當您準備繼續時單擊『下一步』。
FullInstallation=完全安裝
; if possible don't translate 'Compact' as 'Minimal' (I mean 'Minimal' in your language)
CompactInstallation=精簡安裝
CustomInstallation=自定義安裝
NoUninstallWarningTitle= 組件存在
NoUninstallWarning=安裝程式檢測到您的電腦中已經安裝了如下元件：%n%n%1%n%n取消這些組件將不會安裝它們。%n%n您現在要繼續嗎？
ComponentSize1=%1 KB
ComponentSize2=%1 MB
ComponentsDiskSpaceMBLabel= 目前選中的至少需要 [mb] MB 的磁碟空間。
; *** "Select Additional Tasks" wizard page
WizardSelectTasks=選擇額外任務
SelectTasksDesc=還有哪些任務會被執行呢？
SelectTasksLabel2=選擇您想在安裝[name]的過程中執行的其他額外任務，然後單擊『下一步』。
; *** "Select Start Menu Folder" wizard page
WizardSelectProgramGroup= 選擇開始功能表
SelectStartMenuFolderDesc=安裝程式會在哪兒創建程式的快捷方式？
SelectStartMenuFolderLabel3= 安裝程式將在如下的開始功能表檔夾中創建程式的快捷方式。
SelectStartMenuFolderBrowseLabel=單擊『下一步』繼續。如果您要選擇其他檔夾，請單擊『流覽』。
MustEnterGroupName=您必須輸入一個檔夾名稱。
GroupNameTooLong= 檔夾名稱或路徑太長了。
InvalidGroupName=檔夾名稱不合法。
BadGroupName=檔夾名稱不能含有如下字元：%n%n%1
NoProgramGroupCheck2=不創建開始菜單檔夾(&D)
; *** "Ready to Install" wizard page
WizardReady=準備安裝
ReadyLabel1= 安裝程式正準備在您的機器中安裝[name].
ReadyLabel2a=繼續安裝請單擊『安裝』，如果您要回顧或修改一些設置，請單擊『上一步』。
ReadyLabel2b=繼續安裝請單擊『安裝』。
ReadyMemoUserInfo=用戶資訊:
ReadyMemoDir= 目標位置:
ReadyMemoType=安裝類型:
ReadyMemoComponents=選中的組件:
ReadyMemoGroup= 開始菜單夾:
ReadyMemoTasks=附加任務項:
; *** "Preparing to Install" wizard page
WizardPreparing=準備安裝
PreparingDesc= 安裝程式正準備在您的機器中安裝[name].
PreviousInstallNotCompleted=之前有一個安裝/卸載程式尚未完成。您得重新啟動電腦完成該安裝。%n%n重啟機器之後，請再次運行安裝程式以完成[name]的安裝。
CannotContinue=安裝程式無法繼續。請單擊『取消』退出。
; *** "Installing" wizard page
WizardInstalling=安裝中
InstallingLabel= 正在安裝[name], 請等待.
; *** "Setup Completed" wizard page
FinishedHeadingLabel=完成 [name]安裝嚮導
FinishedLabelNoIcons=安裝程式已完成[name]的安裝.
FinishedLabel=安裝程式已經在您的電腦中安裝了[name]。選中安裝後的圖示可啟動應用程式。
ClickFinish=單擊『完成』退出安裝。
FinishedRestartLabel= 為能完成[name]安裝過程, 現在需要重啟機器, 您願意嗎?
FinishedRestartMessage=要完成[name]的安裝，必須重新啟動您的電腦。%n%n您願意現在重啟嗎？
ShowReadmeCheck=是的, 我要閱讀README文件
YesRadio= (&Y)是的, 現在重啟電腦
NoRadio=(&N)不, 我以後再重啟電腦
; used for example as 'Run MyProg.exe'
RunEntryExec=運行 %1
; used for example as 'View Readme.txt'
RunEntryShellExec=閱讀 %1
; *** "Setup Needs the Next Disk" stuff
ChangeDiskTitle=安裝程式需要下一個磁片
SelectDiskLabel2=請插入磁片 %1 並單擊確定。%n%n如果除了以下列出的之外，仍未找到磁片檔的話，請輸入正確路徑或單擊『流覽』。
PathLabel=路徑(& P)：
FileNotInDir2="%2" 中無法定位文件 "%1"。請插入正確磁片或選擇其他檔夾。
SelectDirectoryLabel= 請指定下一張磁片的位置。
; *** Installation phase messages
SetupAborted=安裝未完成。%n%n請檢查問題並重新運行安裝程式。
EntryAbortRetryIgnore=重試請單擊『重試』，忽略將會繼續進行，終止將取消安裝。
; *** Installation status messages
StatusCreateDirs=正在創建檔路徑...
StatusExtractFiles= 正在釋放檔...
StatusCreateIcons=正在創建快捷方式...
StatusCreateIniEntries=正在創建INI入口...
StatusCreateRegistryEntries=正在創建註冊表入口...
StatusRegisterFiles= 正在註冊檔...
StatusSavingUninstall=正在保存反安裝資訊...
StatusRunProgram=正在結束安裝...
StatusRollback=正在恢復到原先的狀態...
; *** Misc. errors
ErrorInternal2=內部錯誤: %1
ErrorFunctionFailedNoCode=%1 失敗
ErrorFunctionFailed=%1 失敗；代碼為 %2
ErrorFunctionFailedWithMessage=%1 失敗；代碼為 %2.%n%3
ErrorExecutingProgram=無法執行檔：%n%1
; *** Registry errors
ErrorRegOpenKey=打開註冊表鍵值錯：%n%1\%2
ErrorRegCreateKey= 創建註冊表鍵值錯：%n%1\%2
ErrorRegWriteKey=寫入註冊表鍵值錯：%n%1\%2
; *** INI errors
ErrorIniEntry=在檔 "%1" 中創建INI入口資訊錯。
; *** File copying errors
FileAbortRetryIgnore=單擊『重試』可重新嘗試一次，忽略可跳過該檔（不推薦），或終止退出安裝。
FileAbortRetryIgnore2=單擊『重試』可重新嘗試一次，忽略可繼續執行（不推薦），或終止退出安裝。
SourceIsCorrupted=原始檔案已損壞
SourceDoesntExist=原始檔案 "%1" 不存在
ExistingFileReadOnly= 已存在的文件為唯讀檔案。%n%n單擊重試可移除唯讀屬性，然後重試。忽略可跳過此檔，終止可取消安裝。
ErrorReadingExistingDest= 嘗試讀檔時有錯誤發生：
FileExists=文件已存在。%n%n想要安裝程式覆蓋它嗎？
ExistingFileNewer=已存在的檔版本較新，建議保留該檔。%n%n您願意保留該檔嗎？
ErrorChangingAttr=嘗試修改檔屬性時有錯誤發生：
ErrorCreatingTemp= 嘗試在目標路徑下創建檔時有錯誤發生：
ErrorReadingSource=嘗試讀原始檔案時有錯誤發生：
ErrorCopying= 嘗試複製檔時有錯誤發生：
ErrorReplacingExistingFile=嘗試替換檔時有錯誤發生：
ErrorRestartReplace=RestartReplace failed:
ErrorRenamingTemp=嘗試重命名目標路徑的檔時有錯誤發生：
ErrorRegisterServer= 無法註冊 DLL/OCX：%1
ErrorRegSvr32Failed=RegSvr32 失敗退出碼 %1
;ErrorRegisterServerMissingExport=未發現DllRegisterServer 的導出介面
ErrorRegisterTypeLib=無法註冊類型庫：%1
; *** Post-installation errors
ErrorOpeningReadme=嘗試打開 README 檔時有錯誤發生。
ErrorRestartingComputer=安裝程式無法重啟電腦，請手動操作。
; *** Uninstaller messages
UninstallNotFound="%1"文件不存在。無法卸載。
UninstallOpenError="%1" 文件無法打開。無法卸載
UninstallUnsupportedVer=該版本的卸載工具無法識別當前卸載日誌"%1"中的格式。無法卸載
UninstallUnknownEntry= 在卸載程式的日誌中遇到不可識別的入口資訊(%1)
ConfirmUninstall=您確定要完全刪除%1以及它的相關元件嗎?
UninstallOnlyOnWin64= 該安裝程式只能在64位元Windows作業系統下被卸載。
OnlyAdminCanUninstall=該安裝程式只能由具有管理員許可權的用戶卸載。
UninstallStatusLabel=%1 將從您的電腦中被卸載，請等待。
UninstalledAll=%1 已成功被卸載。
UninstalledMost=%1 卸載完成。%n%n但仍有些檔未被移除，請您手動刪除它們。
UninstalledAndNeedsRestart= 要完成本次%1的卸載，您必須重啟機器。%n%n您願意現在重啟嗎？
UninstallDataCorrupted="%1" 文件已損壞，無法卸載
; *** Uninstallation phase messages
ConfirmDeleteSharedFileTitle= 要刪除共用檔嗎?
ConfirmDeleteSharedFile2=以下共用檔不再被任何程式使用，您要卸載並刪除這些共用檔嗎？%n%n一旦它被刪除，若有其他程式仍要使用該檔的話，那些程式的功能將會不完整。如果您不確定，請選擇『否』。保留該檔在系統中不會引起任何損害。
SharedFileNameLabel=檔案名稱:
SharedFileLocationLabel=位置:
WizardUninstalling= 卸載狀態
StatusUninstalling=正在卸載 %1...
; The custom messages below aren't used by Setup itself, but if you make
; use of them in your scripts, you'll want to translate them.
[CustomMessages]
NameAndVersion=%1 版本 %2
AdditionalIcons=其他圖示：
CreateDesktopIcon= 創建桌面圖示(&d)
CreateQuickLaunchIcon=創建快速啟動圖示(&Q)
ProgramOnTheWeb=%1 相關網站
UninstallProgram=卸載 %1
LaunchProgram=運行 %1
AssocFileExtension=%1 將與文件副檔名%2建立關聯(&A)
AssocingFileExtension=正在建立%1與檔副檔名%2的關聯關係...
