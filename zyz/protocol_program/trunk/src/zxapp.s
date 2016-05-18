<?xml version="1.0" encoding="UTF-8"?>
<zxapp>
    <zx>
        <server>
            <addr type="ipv4_addr"/>
            <port>
                <tcp type="ipv4_port"/>
                <udp type="ipv4_port"/>
            </port>
        </server>
        <client>
            <username type="uint" range="{(1001,9999]}"/>
            <port type="const_int"/>
        </client>
    </zx>
</zxapp>
