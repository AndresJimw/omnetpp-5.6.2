network wn12
{
    types:
        channel canal extends ned.DelayChannel{ delay = 100ms; }
    submodules:
        box12[6]: box12;
    connections:
        box12[0].gate++ <--> canal <--> box12[1].gate++;
        box12[1].gate++ <--> canal <--> box12[2].gate++;
        box12[1].gate++ <--> canal <--> box12[4].gate++;
        box12[3].gate++ <--> canal <--> box12[4].gate++;
        box12[4].gate++ <--> canal <--> box12[5].gate++;
}

simple box12
{
    parameters:
        @display("i=block/rxtx");
    
    gates:
        inout gate[];
}



