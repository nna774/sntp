# SNTP クライアントの適当な実装。
適当にどんな感じなのか理解するために[rfc2030](http://www.geocities.co.jp/HeartLand-Cosmos/2211/rfc2030.html)([本家](http://www.ietf.org/rfc/rfc2030.txt))
見ながら実装(きたない)。

# こんなかんじです。
````
[nona@sazanami(18:49)] ~/codes/sntp(master)
 $ ./a.out 133.243.238.244
Now: 3639030557
send 48 byte(s)!
header: (0 4 4 1 0 -20)
rootDelay: 0
rootDisp: 0
refIdent: NICT
refTS: 4.89941e+08
origTS: 4.89941e+08
recTS: 4.89941e+08
  refTS:        3639030557, 0
 origTS:        3639030557, 0
  recTS:        3639030557, 818314947
transTS:        3639030557, 818318400

[nona@sazanami(18:52)] ~/codes/sntp(master)
 $ ./a.out 10.8.8.12
Now: 3639031420
send 48 byte(s)!
header: (0 4 4 2 0 -20)
rootDelay: 0
rootDisp: 0
refIdent: (snip)
refTS: 7.24953e+08
origTS: 2.08397e+09
recTS: 2.08397e+09
  refTS:        3639031083, 3062649564
 origTS:        3639031420, 0
  recTS:        3639031420, 393466758
transTS:        3639031420, 393558827
````

# LICENSE
MIT LICENSE
