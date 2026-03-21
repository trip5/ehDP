#- ehDP responder for Tasmota
 - https://github.com/trip5/ehDP
 -
 - Drop this file into the filesystem as `ehdp.be` and it will auto-start.
 - Configure material symbol with: EhDPSymbol 0xe03e
 - Optional: EhDPCapability api (can be called multiple times)
 - 
 - Automatically pulls name, IP, firmware version, and mDNS hostname.
 -
 - Note that this is untested.  Please help if you think you can improve it.  -Trip5
 -#

import udp
import json
import string

class EhDP
    static PORT = 11867
    static REQUEST = "EHDP_DISCOVER"
    static PROTO = "ehdp/1.0"
    
    var sock
    var material_symbol
    var capabilities
    
    def init()
        # Load saved config from persist
        self.material_symbol = persist.find("ehdp_symbol", nil)
        self.capabilities = persist.find("ehdp_caps", [])
        
        # Create and bind UDP socket
        self.sock = udp()
        var result = self.sock.begin("0.0.0.0", self.PORT)
        
        if result
            print(string.format("ehDP: Listening on port %d", self.PORT))
            if self.material_symbol
                print(string.format("ehDP: Symbol = %s", self.material_symbol))
            end
        else
            print("ehDP: Failed to bind UDP socket")
        end
        
        # Register custom commands
        tasmota.add_cmd("EhDPSymbol", / cmd, idx, payload -> self.cmd_symbol(payload))
        tasmota.add_cmd("EhDPCapability", / cmd, idx, payload -> self.cmd_capability(payload))
    end
    
    def every_50ms()
        if !self.sock return end
        
        # Check for incoming packets
        var packet = self.sock.read()
        if !packet return end
        
        var msg = packet[0]  # Message bytes
        var remote_ip = packet[1]  # Sender IP
        var remote_port = packet[2]  # Sender port
        
        # Convert bytes to string
        msg = msg.asstring()
        
        # Trim trailing whitespace
        msg = string.strip(msg)
        
        # Check if it's a discovery request
        if msg == self.REQUEST
            print(string.format("ehDP: Discovery from %s:%d", remote_ip, remote_port))
            self.respond(remote_ip, remote_port)
        end
    end
    
    def respond(remote_ip, remote_port)
        var wifi = tasmota.wifi()
        var device_name = tasmota.cmd("DeviceName")["DeviceName"]
        var hostname = wifi.find("hostname", "tasmota")
        
        # Build JSON response
        var response = {
            "proto": self.PROTO,
            "ip": wifi["ip"],
            "project": "Tasmota",
            "firmware": tasmota.version(),
            "name": device_name,
            "mdns": hostname,
            "ui_port": 80
        }
        
        # Add material symbol if set
        if self.material_symbol
            response["material_symbol"] = self.material_symbol
        end
        
        # Add capabilities if present
        if size(self.capabilities) > 0
            response["capabilities"] = self.capabilities
        end
        
        # Serialize to JSON
        var json_str = json.dump(response)
        
        # Send response
        self.sock.send(remote_ip, remote_port, json_str)
    end
    
    def cmd_symbol(payload)
        if payload == "" || payload == "?"
            if self.material_symbol
                tasmota.resp_cmnd(self.material_symbol)
            else
                tasmota.resp_cmnd("Not set")
            end
        else
            self.material_symbol = payload
            persist.ehdp_symbol = payload
            persist.save()
            tasmota.resp_cmnd_done()
        end
    end
    
    def cmd_capability(payload)
        if payload == "" || payload == "?"
            tasmota.resp_cmnd(json.dump(self.capabilities))
        else
            # Add capability if not already present
            if self.capabilities.find(payload) == nil
                self.capabilities.push(payload)
                persist.ehdp_caps = self.capabilities
                persist.save()
            end
            tasmota.resp_cmnd_done()
        end
    end
end

# Auto-start the driver
var ehdp_driver = EhDP()
tasmota.add_driver(ehdp_driver)

print("ehDP: Driver loaded")
