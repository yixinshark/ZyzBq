<?xml version="1.0" encoding="UTF-8"?>
<zxapp>
    <inst>
        <server>
            <addr type="ipv4_addr" allownull="true" cond="reload"/>
            <port>
                <tcp type="ipv4_port"/>
                <udp type="ipv4_port"/>
            </port>
            <audioAlgo type="uint" discrete="{5,7}" alias="G.711A,G.729A"/>
        </server>
        <client>
            <username type="uint" range="{[1002,9999]}" allownull="true"/>
            <port type="ipv4_port" const="true"/>
        </client>
        <state type="int" discrete="{0,1,2,3}" alias="idle,disconnected,connecting,connected"/>
    </inst>
</zxapp>
