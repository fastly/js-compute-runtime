echo 'with-pbl Richards:' && for a in {1..10}; do curl 'http://localhost:7676/Richards' --connect-to '127.0.0.1:443' --silent ; done
echo 'with-pbl DeltaBlue:' && for a in {1..10}; do curl 'http://localhost:7676/DeltaBlue' --connect-to '127.0.0.1:443' --silent ; done
echo 'with-pbl Crypto:' && for a in {1..10}; do curl 'http://localhost:7676/Crypto' --connect-to '127.0.0.1:443' --silent ; done
echo 'with-pbl RayTrace:' && for a in {1..10}; do curl 'http://localhost:7676/RayTrace' --connect-to '127.0.0.1:443' --silent ; done
echo 'with-pbl EarleyBoyer:' && for a in {1..10}; do curl 'http://localhost:7676/EarleyBoyer' --connect-to '127.0.0.1:443' --silent ; done
echo 'with-pbl RegExp:' && for a in {1..10}; do curl 'http://localhost:7676/RegExp' --connect-to '127.0.0.1:443' --silent ; done
echo 'with-pbl NavierStokes:' && for a in {1..10}; do curl 'http://localhost:7676/NavierStokes' --connect-to '127.0.0.1:443' --silent ; done
echo 'with-pbl Box2D:' && for a in {1..10}; do curl 'http://localhost:7676/Box2D' --connect-to '127.0.0.1:443' --silent ; done
