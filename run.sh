rm testcases/*.ast*
rm testcases/*.ic*
rm testcases/*.spim*
make clean
make
echo 'running sclp on testcases...'
v="$(ls testcases | grep .*\.c)"
for i in $v; do ./sclp testcases/"$i" -ast -icode  ; done;
for i in $v; do mv testcases/"$i".ast testcases/"$i".ast.me  ; done;
for i in $v; do mv testcases/"$i".ic testcases/"$i".ic.me  ; done;
for i in $v; do mv testcases/"$i".spim testcases/"$i".spim.me  ; done;

echo 'running sclp16 on testcases...'
for i in $v; do ./sclp16 testcases/"$i" -ast -icode  ; done;
echo '=========ast============'
for i in $v; do echo '----------' "$i" '-------------------'; diff testcases/"$i".ast testcases/"$i".ast.me  ; done;
echo '=========icode============'
for i in $v; do echo '----------' "$i" '-------------------'; diff testcases/"$i".ic testcases/"$i".ic.me  ; done;
echo '=========spim============'
for i in $v; do echo '----------' "$i" '-------------------'; diff testcases/"$i".spim testcases/"$i".spim.me  ; done;
rm testcases/*.ast*
rm testcases/*.ic*
rm testcases/*.spim*
