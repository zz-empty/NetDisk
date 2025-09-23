# NetDisk - 私有协议文件管理服务器

## 项目概述

NetDisk是一个基于私有协议的文件管理服务器，功能类似于百度网盘。项目采用C/C++开发，使用CMake构建系统，Git进行版本管理。

## 当前状态

**版本**: 1.0.0-alpha (一期开发中)
**状态**: 项目框架搭建完成

## 项目结构

```
NetDisk/
├── server/                 # 服务器端代码
│   ├── src/               # 源代码
│   ├── include/           # 头文件
│   ├── conf/              # 配置文件
│   ├── lib/               # 第三方库
│   ├── obj/               # 编译中间文件
│   ├── test/              # 测试代码
│   └── log/               # 日志文件
├── client/                # 客户端代码（待开发）
├── build/                 # 构建目录（自动生成）
└── README.md              # 项目说明
```

## 功能规划

### 一期功能
- [x] 项目框架搭建
- [ ] 服务器配置加载
- [ ] 线程池管理
- [ ] 基础网络通信
- [ ] 文件操作命令（cd, ls, pwd, puts, gets, rm, mkdir）

### 二期功能
- [ ] 用户密码验证（基于Linux用户系统）
- [ ] 操作日志记录
- [ ] 断点续传功能
- [ ] 大文件传输优化（mmap）

### 三期功能
- [ ] 用户注册系统（MySQL数据库）
- [ ] 虚拟文件表
- [ ] 文件秒传功能
- [ ] MD5文件校验

### 四期功能
- [ ] 长短命令分离
- [ ] Token身份验证
- [ ] 连接超时管理

### 五期功能
- [ ] 多点下载功能
- [ ] P2P协议支持

## 构建说明

### 环境要求
- Linux操作系统
- GCC编译器（支持C11标准）
- CMake 3.10+
- Git

### 构建步骤

1. 克隆项目
```bash
git clone <repository-url>
cd NetDisk
```

2. 配置和构建
```bash
mkdir build
cd build
cmake ..
make
```

3. 运行服务器
```bash
./bin/NetDiskServer ../server/conf/server.conf
```

### 配置文件说明

服务器配置文件 (`server/conf/server.conf`):
```ini
# Server Configuration File
IP = 127.0.0.1
PORT = 8080
THREAD_NUM = 10
FILE_DIR = ./server_files
```

<<<<<<< HEAD
## 开发规范

### Git工作流
- 使用英文提交信息前缀（feat, fix, docs, style, refactor, test, chore）
- 功能开发在分支进行，测试通过后合并到main分支
- 每完成一期功能发布一个版本标签
=======
## 开发流程

### Git分支策略
- `main`分支：稳定版本，只接受合并请求
- `dev`分支：开发集成分支，功能测试通过后合并到此
- `feature/*`分支：功能开发分支，从dev分支创建

### 提交规范
使用约定式提交格式：
- `feat:` 新功能
- `fix:` 修复bug  
- `docs:` 文档更新
- `style:` 代码格式调整
- `refactor:` 代码重构
- `test:` 测试相关
- `chore:` 构建过程或辅助工具变动

### 开发步骤
1. 从dev分支创建功能分支：`git checkout -b feature/模块名称`
2. 开发完成后提交：`git commit -m 'feat: 描述功能'`
3. 推送到远程：`git push origin feature/模块名称`
4. 创建Pull Request到dev分支
5. 代码审查后合并

>>>>>>> feature/config-module

### 代码规范
- 采用增量开发方式，写一部分测一部分
- 命令前缀统一为"CMD_" + 大写命令（如CMD_LS）
- main函数保持简洁，功能逻辑分散到其他文件
- 关键代码添加注释说明

## 技术栈

- **语言**: C/C++
- **构建工具**: CMake
- **版本控制**: Git
- **网络通信**: Socket编程
- **并发处理**: 线程池/Epoll
- **数据存储**: 文件系统 + MySQL（后期）

## 贡献指南

1. Fork本项目
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建Pull Request

## 许可证

本项目采用MIT许可证。详见LICENSE文件。

## 联系方式

- 项目仓库: [GitHub链接]
- 问题反馈: [Issues页面]

---

*此README将根据项目进展持续更新。当前内容基于一期开发初期的状态。*
<<<<<<< HEAD
=======

>>>>>>> feature/config-module
