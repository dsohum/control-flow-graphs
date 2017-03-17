// use store_token_name(<TOKEN>) to correctly generate tokens file
%filenames="scanner"
%lex-source="scanner.cc"

digit		[0-9]
letter 		[a-zA-Z_]
operators   [-+*/]

%%
void	{
			//printf("keyword token found: %s\n",matched().c_str());
			Scanner::store_token_name("VOID");
			return Parser::VOID;
		}		
int 	{
			//printf("keyword token found: %s\n",matched().c_str());
			Scanner::store_token_name("INTEGER");
			return Parser::INTEGER;
		}
float   {
			//printf("keyword token found: %s\n",matched().c_str());
			Scanner::store_token_name("FLOAT");
			return Parser::FLOAT;
		}
while   {
			//printf("keyword token found: %s\n",matched().c_str());
			Scanner::store_token_name("WHILE");
			return Parser::WHILE;
		}

do      {
			//printf("keyword token found: %s\n",matched().c_str());
			Scanner::store_token_name("DO");
			return Parser::DO;
		}

if      {
			//printf("keyword token found: %s\n",matched().c_str());
			Scanner::store_token_name("IF");
			return Parser::IF;
		}

else    {
			//printf("keyword token found: %s\n",matched().c_str());
			Scanner::store_token_name("ELSE");
			return Parser::ELSE;
		}
for		{
			//printf("keyword token found: %s\n",matched().c_str());
			Scanner::store_token_name("FOR");
			return Parser::FOR;		
		}
switch	{
			//printf("keyword token found: %s\n",matched().c_str());
			Scanner::store_token_name("FOR");
			return Parser::SWITCH;
		}
default	{
			//printf("keyword token found: %s\n",matched().c_str());
			Scanner::store_token_name("FOR");
			return Parser::DEFAULT;	
		}
case	{
			//printf("keyword token found: %s\n",matched().c_str());
			Scanner::store_token_name("FOR");
			return Parser::CASE;
		} 	 
break	{
			//printf("keyword token found: %s\n",matched().c_str());
			Scanner::store_token_name("FOR");
			return Parser::BREAK;
		}
=		{
			//printf("some assignment token found: %s\n",matched().c_str());
			Scanner::store_token_name("ASSIGN_OP");
			return Parser::ASSIGN;			
		}
{operators}		{
			//printf("op token found: %s\n",matched().c_str());	
			Scanner::store_token_name("ARITHOP");		
			return matched()[0];
		}
{digit}+	{
			//printf("integer constant token found: %d\n",atoi(matched().c_str()));
			ParserBase::STYPE__ *val = Scanner::getSval();
			val->integer_value = atoi(matched().c_str());
			Scanner::store_token_name("NUM");
			return Parser::INTEGER_NUMBER;
		}
(({digit}+"."?{digit}*)|({digit}*"."?{digit}+))([eE][+-]?{digit}+)?		{	
			//printf("float constant token found: %f at %d\n",atof(matched().c_str()),lineNr());
			//([eE][+-]?{digit}+)?
			ParserBase::STYPE__ *val = Scanner::getSval();
			val->double_value = atof(matched().c_str());
			Scanner::store_token_name("FNUM");
			return Parser::DOUBLE_NUMBER;
		}

{letter}({letter}|{digit})* {
			//printf("identifier token found: %s\n",matched().c_str());
			ParserBase::STYPE__ *val = Scanner::getSval();
			val->string_value = new string(matched());
			Scanner::store_token_name("NAME");
			return Parser::NAME;
		}
"<="	{
			//printf("some token found: %s\n",matched().c_str());
			Scanner::store_token_name("LE");
			return Parser::LE;				
		}
"<"	{
			//printf("some token found: %s\n",matched().c_str());
			Scanner::store_token_name("LT");
			return Parser::LT;				
		}
"=="	{
			//printf("some token found: %s\n",matched().c_str());
			Scanner::store_token_name("EQ");
			return Parser::EQ;				
		}
"!="	{
			//printf("some token found: %s\n",matched().c_str());
			Scanner::store_token_name("NE");
			return Parser::NE;				
		}
">="	{
			//printf("some token found: %s\n",matched().c_str());
			Scanner::store_token_name("GE");
			return Parser::GE;				
		}
">"		{
			//printf("some token found: %s\n",matched().c_str());
			Scanner::store_token_name("GT");
			return Parser::GT;				
		}
"||"		{
			//printf("some token found: %s\n",matched().c_str());
			Scanner::store_token_name("OR");
			return Parser::OR;				
		}
"&&"		{
			//printf("some token found: %s\n",matched().c_str());
			Scanner::store_token_name("AND");
			return Parser::AND;				
		}
"!"		{
			//printf("some token found: %s\n",matched().c_str());
			Scanner::store_token_name("NOT");
			return Parser::NOT;				
		}

"{"		|
"}"		|
;		|
"("		|
")"		|
:		|
"?"		|
","		{
			//printf("some token found: %s\n",matched().c_str());
			Scanner::store_token_name("META CHAR");
			return matched()[0];
		}

\n    		|
";;".*  	|
[ \t]*";;".*	|
[ \t]*"//".*	|
[ \t]		{
			//printf("some ignored token found: %s\n",matched().c_str());
			if (command_options.is_show_tokens_selected())
				ignore_token();
		}

.		{ 
			string error_message;
			error_message =  "Illegal character `" + matched();
			error_message += "' on line " + lineNr();
			
			CHECK_INPUT(CONTROL_SHOULD_NOT_REACH, error_message, lineNr());
		}
