//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
simple box5 {
    parameters:
        bool sendMsgOnInit = default(false);
        int limit = default(2);
        @display("i=block/routing");
    gates:
        input in;
        output out;
}

simple box5a extends box5 {
    parameters:
        @display("i=,gold");
        sendMsgOnInit = default(true);
}

simple box5b extends box5 {
    parameters:
        @display("i=,gold");
        sendMsgOnInit = default(true);
}

network wn5
{
    @display("bgb=354,108");
    submodules:
        box5a1: box5a{
        parameters:
            sendMsgOnInit = true;
	        @display("i=,cyan");        
        };
        box5b1: box5b{
        parameters:
            sendMsgOnInit = false;
	        @display("i=,red");        
        };
    connections:
        box5a1.out --> {delay = 100ms;} --> box5b1.in;
        box5b1.out --> {delay = 100ms;} --> box5a1.in;
}