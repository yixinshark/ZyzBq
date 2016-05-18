<?xml version="1.0" encoding="UTF-8"?>
<zxapp>
    <zx inst="true">
        <server>
            <addr>"192.168.200.200"</addr>
            <port>
                <tcp>4000</tcp>
               <!----交互得到服务器udp接收端口号---> 
                <udp/>
            </port>
        </server>
        <client>
            <username>2004</username>
         <!-- udp 本地端口，固定设置 -->
            <port>3000</port>
        </client>
    </zx>
</zxapp>
