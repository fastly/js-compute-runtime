echo 'no-pbl Richards:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-no-pbl.edgecompute.app/Richards' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary
echo 'no-pbl DeltaBlue:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-no-pbl.edgecompute.app/DeltaBlue' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary
echo 'no-pbl Crypto:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-no-pbl.edgecompute.app/Crypto' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary
echo 'no-pbl RayTrace:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-no-pbl.edgecompute.app/RayTrace' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary
echo 'no-pbl EarleyBoyer:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-no-pbl.edgecompute.app/EarleyBoyer' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary
echo 'no-pbl RegExp:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-no-pbl.edgecompute.app/RegExp' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary
echo 'no-pbl NavierStokes:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-no-pbl.edgecompute.app/NavierStokes' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary
# echo 'no-pbl Mandreel:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-no-pbl.edgecompute.app/Mandreel' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary
# echo 'no-pbl CodeLoad:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-no-pbl.edgecompute.app/CodeLoad' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary
echo 'no-pbl Box2D:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-no-pbl.edgecompute.app/Box2D' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary
# echo 'no-pbl Typescript:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-no-pbl.edgecompute.app/Typescript' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary



echo 'with-pbl Richards:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-with-pbl.edgecompute.app/Richards' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary
echo 'with-pbl DeltaBlue:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-with-pbl.edgecompute.app/DeltaBlue' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary
echo 'with-pbl Crypto:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-with-pbl.edgecompute.app/Crypto' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary
echo 'with-pbl RayTrace:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-with-pbl.edgecompute.app/RayTrace' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary
echo 'with-pbl EarleyBoyer:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-with-pbl.edgecompute.app/EarleyBoyer' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary
echo 'with-pbl RegExp:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-with-pbl.edgecompute.app/RegExp' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary
echo 'with-pbl NavierStokes:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-with-pbl.edgecompute.app/NavierStokes' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary
# echo 'with-pbl Mandreel:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-with-pbl.edgecompute.app/Mandreel' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary
# echo 'with-pbl CodeLoad:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-with-pbl.edgecompute.app/CodeLoad' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary
echo 'with-pbl Box2D:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-with-pbl.edgecompute.app/Box2D' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary
# echo 'with-pbl Typescript:' && ssh cache-sql13425 "for a in {1..10}; do curl 'https://pperf-with-pbl.edgecompute.app/Typescript' --connect-to '127.0.0.1:443' --silent ; done" | tee "$(tty)" | st --summary
