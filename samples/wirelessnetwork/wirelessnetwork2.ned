simple box2
{
    gates:
        input in;
        output out;
		
}

network wn2
{
    submodules:
        box2a: box2;
        box2b: box2;
    connections:
        box2a.out --> {delay = 100ms;} --> box2b.in;
        box2b.out --> {delay = 100ms;} --> box2a.in; 
}









