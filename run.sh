rm testcases/*.ast*
rm testcases/*.ic*
rm testcases/*.spim*
rm testcases/*.dce*
make clean
make
echo 'running sclp on testcases...'
v="$(ls testcases | grep .*\.c)"
for i in $v; do (>&2 echo '----------' "$i" '-------------------'); echo '----------' "$i" '-------------------'; ./sclp testcases/"$i" -ast -icode  ; done;
for i in $v; do mv testcases/"$i".ast testcases/"$i".ast.me  ; done;
for i in $v; do mv testcases/"$i".ic testcases/"$i".ic.me  ; done;
for i in $v; do mv testcases/"$i".dce testcases/"$i".dce.me  ; done;

echo 'running sclp_gullu on testcases...'
for i in $v; do ./sclp_gullu testcases/"$i" -ast -icode  ; done;
# echo '=========ast============'
# for i in $v; do echo '----------' "$i" '-------------------'; diff testcases/"$i".ast testcases/"$i".ast.me  ; done;
echo '=========icode============'
for i in $v; do echo '----------' "$i" '-------------------'; diff testcases/"$i".ic testcases/"$i".ic.me  ; done;
echo '=========dce============'
for i in $v; do echo '----------' "$i" '-------------------'; diff testcases/"$i".dce testcases/"$i".dce.me  ; done;
# rm testcases/*.ast*
# rm testcases/*.ic*
# rm testcases/*.spim*
# rm testcases/*.dce*
