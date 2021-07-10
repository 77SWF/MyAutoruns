

**自启动项的研究与软件实现**

# 目录

* [设计要求](#设计要求)
* [原理、实践与实现思路](#原理实践与实现思路)
* [详细设计与开发](#详细设计与开发)
  * [概述](#概述)
  * [开发环境](#开发环境)
  * [图形界面](#图形界面)
  * [主要功能函数](#主要功能函数)
    * [注册表读取](#注册表读取)
    * [字符串处理](#字符串处理)
    * [计划任务文件读取](#计划任务文件读取)
    * [提取签名信息](#提取签名信息)
  * [Logon实现](#logon实现)
    * [自启动目录实现](#自启动目录实现)
    * [注册表实现](#注册表实现)
    * [代码流程](#代码流程)
    * [结果演示](#结果演示)
  * [Services实现](#services实现)
    * [代码流程](#代码流程-1)
    * [结果演示](#结果演示-1)
  * [Drivers实现](#drivers实现)
    * [代码流程](#代码流程-2)
    * [结果演示](#结果演示-2)
  * [Scheduled tasks实现](#scheduled-tasks实现)
    * [代码流程](#代码流程-3)
    * [结果演示](#结果演示-3)
  * [Known Dlls实现](#known-dlls实现)
    * [代码流程](#代码流程-4)
    * [结果演示](#结果演示-4)



# 设计要求

1. 结合课堂所讲授的内容，围绕着“Windows 自启动项的查看和分析”主 题，查阅课外资料，编写验证代码，动手进行实验，撰写一份技术研究和设计开发报告。
2. 可以参考 `SysinternalsSuite` 工具集中的 `Autoruns` 软件，了解在 Windows 系统中有哪些可以实现自启动的技术方法，然后分析它们各自的技术原理、实现 细节和隐蔽性状况，撰写到课程报告之中。
3. 编写自己的 Windows 自启动项查看软件。

# 原理、实践与实现思路

&emsp;&emsp;自启动是许多木马会使用的技术，如挖矿木马、远控木马等，因此，检测自启动项是除了测试网络连接外的检测木马的重要技术。常见有以下三类自启动技术，其中，本报告主要分析的是前两类：

1. 随着计算机启动而启动
2. 随用户登录而启动
3. 随软件运行而启动

&emsp;&emsp;下面列举了属于前两大类的几种自启动技术，其中，打钩的是我在设计实现中完成的几种，并将在后面进行原理的研究分析：

- [x] Logon：启动目录，基于注册表启动
- [x] Services：系统服务
- [x] Drivers：系统驱动程序
- [x] Scheduled Tasks：计划任务
- [ ] Internet Explorer：IE 浏览器的 BHO 对象 
- [ ] Boot Execute：启动执行
- [ ] Image Hijacks：映像劫持
- [x] Known DLLs：知名动态链接库
- [ ] Winsock Providers：Winsock 服务提供程序
- [ ] Winlogon：用户登录通知程序

&emsp;&emsp;考虑到有些实践需要重启电脑，方便起见，虽然我的代码实现在win10上，但是报告内的实践部分截图来自win7虚拟机。



--删除--



# 详细设计与开发

## 概述

   结合各种自启动方法的**原理分析**，我在每一部分下已经写出了根据原理得到的**实现的基本思路**，因此这部分将主要说明**代码的具体实现**。

   此外，我还参考 `SysinternalSuit` 工具集中的自启动项查看软件 `Autoruns.exe`，并查阅[msdn](https://docs.microsoft.com/zh-cn/)上的相关API的使用资料，使用C++编写了自己的Windows自启动项查看的命令行程序，由于时间关系，尚未实现QT界面的编写。

   目前，我实现了以下自启动项种类的查看：

- [x] Logon：启动目录，基于注册表启动
- [x] Services：系统服务
- [x] Drivers：系统驱动程序
- [x] Scheduled Tasks：计划任务
- [x] Known Dlls：知名动态链接库

## 开发环境

|  环境/语言   |             版本              |
| :----------: | :---------------------------: |
|   测试系统   |   Win7 专业版/Win10 家庭版    |
| 编程(译)平台 | Qt creator 4.11.1(32位社区版) |
|   编程语言   |              C++              |
|   GUI实现    |     QT 5.14.2 (msvc环境)      |

## 图形界面

   首先用 `QT Designer` 设计大致的图形界面：

![image-20210609195150347](README.assets/image-20210609195150347-1624629605933.png) 

   程序启动时先初始化表格的每列宽度，然后默认先显示 `logon` 自启动项：

```c++
// 入口函数
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //初始化表列宽:entry descryption publisher imagepath timestamp
    ui->autoruns_table->setColumnWidth(0, 10);
    ui->autoruns_table->setColumnWidth(1, 180);
    ui->autoruns_table->setColumnWidth(2, 180);
    ui->autoruns_table->setColumnWidth(3, 180);
    ui->autoruns_table->setColumnWidth(4, 600);
    ui->autoruns_table->setColumnWidth(5, 180);

    //logon自启动项
    set_logon_table();
}
```

   接下来开始分析为每个按钮编写对应的自启动项查找与显示函数。

## 主要功能函数

## Logon实现

点击 `Logon` 按钮，程序查找并罗列相关的自启动项，主要有：

1. 自启动目录
2. 注册表自启动项

### 自启动目录实现

### 注册表实现

### 代码流程

   自启动目录的代码逻辑简单，在上面直接列出。

   自启动注册表的读取流程如下，具体的函数里实现了**注册表重定向的禁止操作**，详见主要功能函数的介绍。

### 结果演示

   刚打开程序默认显示Logon项，或点击Logon按钮显示以下内容。红框部分是自启动目录，其余是自启动注册表项。

   成功实现了签名的验证，但存在问题：只有 `exe` 可验证，放空的部分表示没有经过验证（而非验证不通过），因为我仅在 `msdn` 上找到了验证PE `.exe` 文件签名验证的函数 `VerifyEmbeddedSignature` ，还没有找到对 `.dll` 文件的验证 `API`，本应通用的 `WinVerifyTrust` 相关的应用也都进行了尝试（因失败全在代码中注释掉了），都以失败告终。

   Wow6432路径下的自启动项的重定向也得到了解决，见红框下的这部分。

   此外，`publisher` 栏由于不明原因无法由 `imagepath` 得到publisher信息，在传入的参数 `imagepath` 确定已经标准化的前提下，调用 `msdn` 提供的函数后依旧无法返回正确文本内容 ，考虑到上课时提过publisher信息是很容易伪造的，重要的是验证是否通过，因此只显示验证结果。

<img src="README.assets/image-20210613153236380-1624629605934.png" alt="image-20210613153236380" style="zoom:50%;" />

【更新2021年6月13日18:33:08】现在可以得到publisher了，验证功能做了优化。

<img src="README.assets/image-20210613183403055-1624629605934.png" alt="image-20210613183403055" style="zoom:50%;" />



   对比下图 `autoruns` 软件的结果，基本可以对应上，基本功能得到实现：

<img src="README.assets/image-20210610214945939-1624629605934.png" alt="image-20210610214945939" style="zoom:50%;" />

## Services实现

### 代码流程

   `Services` 和 `Drivers` 的实现仅在对共享服务进程可执行文件路径的处理上，其他几本相似，所以把流程图画到一起。

### 结果演示

   点击按钮后显示如下图，每列可以拖动，为了显示较长的路径信息，时间列没有显示完全。实现了共享进程的dll真实imagepath读取。

【更新2021年6月13日18:33:08】现在可以得到publisher了。

 <img src="README.assets/image-20210613183523280-1624629605935.png" alt="image-20210613183523280" style="zoom:50%;" />

   对比下图 `autoruns` 软件，基本可以对应，基本功能得到实现：

<img src="README.assets/image-20210610231226057-1624629605935.png" alt="image-20210610231226057" style="zoom:50%;" />

## Drivers实现

### 代码流程

   见 `3.6.1` Services的代码流程。

### 结果演示

【更新2021年6月13日18:33:08】现在可以验证签名了，但sys文件的验证依旧只有一小部分可以验证。

   点击 `Drivers` 按钮，显示如下。~~这里都是 `.sys` 文件，因此都没有实现签名验证，但是~~基本的 `imagepath` 和部分`description` 等内容读取成功。

<img src="README.assets/image-20210613183704012-1624629605935.png" alt="image-20210613183704012" style="zoom:50%;" />

   对比 `autoruns` 软件，基本可以对应，基本功能得到实现：

<img src="README.assets/image-20210610231302998-1624629605935.png" alt="image-20210610231302998" style="zoom:50%;" />

## Scheduled tasks实现

### 代码流程

### 结果演示

   点击按钮，结果如图所示，和 `autoruns` 对比后，存在的问题是错误地判断了很多应该通过验证的exe文件，目前不清楚原因，因为调用的是 `msdn` 提供的可以验证PE文件签名的函数，在 `Logon` 里的验证结果正确性也很高。

   `imagepath` 中有几个乱码，经过查看，应该是中文字符"新建文件夹"，因为Qt对字符编码的某些原因导致无法正常显示。

【更新2021年6月13日18:33:08】更新签名的验证。

<img src="README.assets/image-20210613183729377-1624629605935.png" alt="image-20210613183729377" style="zoom:50%;" />

## Known Dlls实现

### 代码流程

   这部分几乎和 `Logon` 的实现一样，但只需读取一个子键下的键值，且由于 `Known Dlls` 的注册表键值里的数据都是dll文件而不是exe文件，我暂时无法实现对签名的验证。

### 结果演示

   点击按钮，结果如图所示，暂时无法从dll文件获取到签名验证信息：

【更新2021年6月13日18:33:08】更新签名的验证。

<img src="README.assets/image-20210613183755138-1624629605936.png" alt="image-20210613183755138" style="zoom:50%;" />

