void main();

main()
{
	int a, c,i;
	float b;
	a = 1;
	c = 2;
	b = 1.1;

	if(a <  1)
		b = 1.2;
	a = a + a<c?a:c;
	while(a < 1)
		a = 2;
	a = 0;

	do {
		a = a + 1;
	}while (a < 1);

	while(a < 2)
	{
		a = a+1;
	}
	a = 1;
	for (i = 1 ; i < 3; i = i + 1)
		a = a + 1;

	for(a=0,a=0;a<1;a=a+1){
		a=1;
	}

	for(;a<2;){
		a=a+1;
	}

	for(;a<3;a=a+1,a=a+1){
		a=a+1;
	}


	switch ( a ) {
		case 1: a=1; break;
		case 2: a=2; break;
		default : a=3; 
	}
	switch ( a ) {
		default : a=3; 
	}
	switch ( a ) {
		case 1: a=1; break;
	}

}
