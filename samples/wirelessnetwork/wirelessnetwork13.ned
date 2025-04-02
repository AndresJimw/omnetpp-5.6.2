network wn13
{
    types:
        channel canal extends ned.DelayChannel{ delay = 100ms; }
    submodules:
        box13[6]: box13;
    connections:
        box13[0].gate++ <--> canal <--> box13[1].gate++;
        box13[1].gate++ <--> canal <--> box13[2].gate++;
        box13[1].gate++ <--> canal <--> box13[4].gate++;
        box13[3].gate++ <--> canal <--> box13[4].gate++;
        box13[4].gate++ <--> canal <--> box13[5].gate++;
}

simple box13
{
    parameters:
        @display("i=block/rxtx");
    
    gates:
        inout gate[];
}



