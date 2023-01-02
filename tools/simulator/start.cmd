if ["%~1"]==["-clipv2"] (
node server/main.js -clipv2
) else (
node server/main.js
)