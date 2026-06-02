# 嵌入式综合实验

## 注意事项

* 使用 xubuntu18 虚拟机。
* 关于 pass.c 的路径，详细参考实验手册，本人不负责这部分
* /home/uptech 目录下创建 expTest 目录；其中放置本仓库 expTest 目录中除 pass.c 的其余全部文件

## 步骤
1. 配置主机 Linux 环境。在虚拟机中以 root 方式登录，密码为 123456，虚拟机设置为桥接方式。
2. 在虚拟机中修改文件“/etc/netplan/01-network-manager-all.yaml”并执行命令 `netplan apply`。然后使用命令 `ifconfig ens33`再次查看ip，确认已改为 192.168.50.128。
3. 修改本机防火墙设置，仅开启公用网络防火墙
4. 连接串口与网线，并将网线设置为 100M + 全双工
5. 进入超级终端，连接串口，波特率设置为 115200， parity 设置为 null
6. 打开实验箱电源，启动成功后执行命令 `ifconfig eth0 192.168.50.100 netmask 255.255.255.0`。然后再次执行指令 `ifconfig eth0` 观察 ip是否已被置为 192.168.50.100
7. 在超级中端中执行指令 ping 192.168.50.128 观察是否能连接上虚拟机。连接上则无误
8. 在虚拟机上执行指令，生成可执行文件。命令如下所示：
  * `cd /home/uptech/expTest`
  * `source/opt/fsl-imx-wayland/4.9.88-2.0.0/environment-setup-cortexa9hf-neon-poky-linux-gnueabi`
  * `/opt/fsl-imx-wayland/4.9.88-2.0.0/sysroots/x86_64-pokysdk-linux/usr/bin/qt5/qmake`
  * `make`
9. 将uptech 目录挂载至目标机，即实验箱上。执行以下命令：
  * `mount -t nfs 192.168.50.128:/home/uptech /mnt`
10. 执行目标文件。命令如下：
* `cd /mnt/expTest`
* `./expTest`
11. 在目标机上执行以下命令：
* `cd /mnt/boa`
* `./boa`
12. 在虚拟机上打开浏览器，输入 192.168.50.100 即可访问 html 页面
