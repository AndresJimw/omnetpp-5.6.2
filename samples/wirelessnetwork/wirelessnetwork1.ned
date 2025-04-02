simple box1
{
    gates:
        input in;
        output out;
		
}

network wn1
{
    submodules:
        box1a: box1;
        box1b: box1;
    connections:
        box1a.out --> {delay = 100ms;} --> box1b.in;
        box1b.out --> {delay = 100ms;} --> box1a.in; 
}









