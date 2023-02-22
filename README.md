# ICS-PJ-Backend

A Y86-64 CPU Simulator (FUDAN ICS 2022Fall Project)

This repo is a part of the whole Project. 
Compiled as an executable, it can read in `.yo` file and output running log in json format to stdout, which is the first part of the Project request.

The full power of this simulator can only be access with the front-end part of the project, but my partner didn't release it on github. 2333

I provide a series of APIs to enable real-time interact between front-end and back-end. In front-end part, we provide various function such as 'step', 'revoke', 'send immediate instruction' and so on. 
Most importantly, the simulator has 2 modes, one of which is PIPELINE mode. We simulate the behavior of a 5-level pipeline CPU, and all the value of PIPELINE register will be showed with nice UI. When a Pipeline register received STALL or BUBBLE signal, its UI info box will change its color, which is showed below:

![UI.jpg](https://i.imgtg.com/2023/02/22/st7LM.jpg)

## Usage

```sh
$ make
$ build/y86_sim < case/prog1.yo > result.json
```

## Project Structure

```text
ICS-PJ-Backend
├── obj             # 存放目标文件 (make自动生成)
├── bin             # 存放二进制文件 (make自动生成)
├── debug           # 存放调试用目标文件和二进制文件 (make自动生成)
├── test            # 存放测试用程序
├── case            # 存放测试用例
├── src             # 存放源码
    └── include         # 存放第三方库
├── dll             # 存放用于生成dll的VS项目
├── Makefile
└── README.md       # 项目说明
```
