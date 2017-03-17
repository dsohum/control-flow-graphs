void main(); main()
{
	int a;
	int b;
	float c;
	float x;
	float y;
	//check float and int computations
	a = 0;
	b = 1;
	x = 3.0;
	y = 9.9;

	area = x * y;
	perimeter = 2.0 * (x + y) - 0.0;

	area = 22.0 * x * x / 7.0;
	perimeter = 2.0 * 22.0 * x /7.0;

	//int,float reg and conditional
	a = (a+1>b+2)?(x+3.0>y+4.0||x>6.0)?a:b:b;


	//check arbitrary nestings of different constructs for proper labelling and register allocation
	while(a>b&&b>1){
		//reg and labels with if else if
		if(a==1||a!=2){
			a=(c>0.1)?(c>0.2)?a:a:a;
		} else if(a!=3){
			a=(c>0.3)?(c>0.4)?4:4:4;
		} else if(a==5||a==6){
			a=7;
		} else {
			a=8;
		}

		//for while
		while(a==9||a==10)
			if(a==11){
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				// a=4; if(a==3){ 
				//deep nesting raises error/ not enough free registers .. as they are freed after compiling else ?
				a=12;
				// }}}}}}}}}}}}}}}}}}}}}}}}}}
			}
			//checking al boolean expr
			else if(!(a==14) && a<15 || a>16){
  				a=-a-a--a;
  			}
			else {
				a=17;
				//for do while
				do{	a=b+1;
				do{	a=b+2;
				while(a>b+2){
					a=b+3;		
					a=(c>0.5)?(c>0.6)?a:a:a;
				 	if(a==18)
						a=19;
				 	else if(a==20)
						a=21;
				}	
				}while(a>b+1);
				}while(a>b);	
			}

	}
}
