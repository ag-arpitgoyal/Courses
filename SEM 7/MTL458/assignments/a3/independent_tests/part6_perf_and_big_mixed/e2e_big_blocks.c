/* Generated test: large requests: multi-64K regions, growth, worst fit growth, big realloc moves
 * Operations are encoded in the string table below and replayed by a small
 * interpreter. Every allocation is filled with a per-allocation byte pattern
 * that is verified before free/realloc, so overlapping blocks or lost data
 * show up as "data corrupted" lines (never expected). */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

#define NH 158
static unsigned char *P[NH];
static size_t N[NH];
static unsigned char T[NH];
static unsigned char statbuf[256];

static const char *const CH[] = {
    "10-va311dukJ600tza3600Zdb330611hkh2G43112ehi313eWYG14hxU15YtuI16yyc23347z1818es2L3aqe2-63519waiH1Abc"
    "gK38327606AAdwh2373A391Bpi17B0341CfM6CCy13C1Dhgt49704EW143E6DDy-m15____________F21Fmo11GtIF31Hep23H1"
    "Ixk16GGoP3I1Jjxs11Kbla26DDy-m11Lwg23L3F1MXC1NXbu31O-veD1PdXz13J3K1Qzw26QQzX29008Rpe21Syyp1LKoae103M6"
    "SSlql6KO03NLKpfZ201TzxsA3R4Us12B4Vw116UW1pB3Q1X1sq23P1Y1bgd26VZ1c61a1vp21b1ulo23a19Q04c1Y183Y13b13Z1"
    "Lb1aQ-61d1bqz81e1yY21f1ebYJF16X1g1h_73f11h1-noE3G9g1_16h1h1kkh23S4i1o1C1j1gYbB1k1jdXG1l1xnO3c16g1m1v"
    "aW91n1bw13D3h13W13n11o1yla43k13l19101p1vZ-36o1o1fdb23p13d16TTzxsA1q1bc14r1224s1O26o1o1-ib21t1i_11u1d"
    "rc21v1zj13j11w1bZ23T6o1o1pob23t13m11x1ez11y1y_x49o1G3s11z1sK1-1ndfK9E03e16z1z1d-11_1goW23w13i1F16o1o"
    "1dZc4I1HG21W2gs13q16o1X2WgoC3v11Y2zsO3W21Z2rn-13-16x1x1jo11a2skY43y17Y201b2xc26a2c2Z-a89u1Wjj43b23O8"
    "d2qmZ24e2x12C206x1x1uq16Z2Z2lvd14f2R43c21g2oj-11h2j71i2ipc61j2m-pK1k2rq_26f2l2bC3g21m2ruw83i23l23d23"
    "k26z1n2fjhB6e2e2XC1o2tY11p2pR3u13r13o26x1x1og31q2iql13n21r2Zlu36m2m2WfeF1s2_t23X21t2iqb43e23j2C4u2o1"
    "C1v2pB3p26v2v2pB3Z24w2v181x2Xkz33x21y2azdB3_16r2z2_pu33s21-2ekyH3v21_2ah11W3lhW44X3w186m2m2WfeF6-2-2"
    "WfeF1Y3qW-81Z3xri36q2q2-c11a3kL6w2w2qI3h26Y3Y3ojd23x13W34b3S44c3u1O4d3c143q21e3yB1f3py11g3_uX11h3rYd"
    "33t23X3KZ343b37m203c31i3xZ23i34j3R46u2k3mb16-2-2WfeF6Z3l3uYcA6h30rbk6C11to13l36Y3Y3uxN13_pgAG0303Y3A"
    "-2m1W66a35WfeF18vvn29X2W391_reX112aI6w2w2rJ9O03g36f3f3gw316jgm11m3qhw81n3Xrx1333k33d39_2Wv23j33m347z"
    "1C6_2_2dm13n33z26_2AncvA49M2K-247f303e31BhrU84gWa31ChYt9I2H6AEWfeF4Hu1OG41Iitg21LruZ23CE3431655ilW63"
    "y23L1Fybc41Jxg14KT46-2-2WfeF3K4MB1KJ76HNti137326JJxg14RR41UYen51Qpp23F699i16UPlmn53-2391VW_xI3N3R6MM"
    "m14a17C1Y1mf26JJhe16JJxj19q1078WWaWWW16w2w2ZZ11b1u_1KQ56JJxj11Z1yH1X1YjT1f1ght23X1359w1W31GyZl31SgZQ"
    "3a13J3S1c1pg2A6_c1gV9Z1Wd1F21g1zyZ11Df--23b16PPwqn5666jgm11h1i51o3kbf13I3E3g11W1qA1n1gpt31k1lea23o31"
    "l1-XbF1p3igl11q3oZk34r3p141s3bX1APm1q41t3cWt84u3Ta31v3xz11w3aak84x3b1O4y3a1C3B1p1jqaAAW30H9p201z3oy-"
    "33q33V1d1oj21j1_w11-3vnj22XWWWWW17W103r34TJ13f1K-361t1elZ11m1qV1_3cidA1W4bz24X4v14F33m11s1imh1F16_3_"
    "3_L1Y4j_mI6l1l1nfbF3z31e1af23D3Y43s1361w1ldb21i1oL688xnM6x3x3oH3Y11q1ZA6w1o1Xeb21v1wxc43k16w2W2yYc36"
    "v1-1_qqC3-11y1uj1688YtZ23d13P3_31Y2lt-I2____________F4a2j1O6h1h1_51b2WWr33G5XWWWWW4WWWWWW41OfxZF3x31"
    "c2wpw8I3H6s3s3WE1f2gI6W2W2Xgc33-33p11g2aewF1i2vl14l2A83g23W41d2hd11k2ssZ93w31z1atxC1Z4Xnu23c29Y306y1"
    "y1Xr11u1yczD3n11r1djj31a4Xo2G3Kt113j13l2C1o2eeX36o1n2hlb2G13t16Z1Z1XK1X2iwt86W2W2xaY16y3e2qL3X46QQyf"
    "z31j2xK3e1C6a2a2nD3u34p2H14Z2OC6l1l1nfbF3M6p2p2H4x21G1_1gqa46n2n2vtb21b4zbqE1c4fhl26v3v3_61d4g91e4sy"
    "j24f4e1O6v3v3cS1g4dfs73p2Ah1m1wJ1r2hS6q1q1tU6v3v3hO1h4qrv36W2W2w-33j21s2cD1i4lyi21j4-pPLj2t4_61k4ltl"
    "21l4r-k43O1v2-dkJ3i16f4h2WfeF1x1lla36x2x2c19d303h16p3p3XxF6QQjxe23c14W3W2a33b41q2nm11t2eldA1m4gn_13_"
    "16k2k2iuU1X3sXV9K03b21b3aiaA1n4-k2G01o4-urF1p4tuc41q4r_q34r4643X31m2kep2Ar4m1jM3q41c3WplA1s4ptiG6u1u"
    "1bzk13o21i3puk23Z43b33s43q23c31u2sXtF3n21Z3qud81h3ehl23k26Ql3WoiB10ccW21Y3Zd28a3uas36x1x1cH8g3_LF21t"
    "4mK4u4m117Z2XWWWWW19L03s23l43r16f2f2o513gml1G01k3ajgA3X23h26y1y1Xr13m43W23r43Y2E6W3W3el6I4HI5J3h43c4"
    "1d3k86l3j3WfeF6Z1Z1aF6W3W3jm61m3qv11n3elf273WWaWWW13n4306r2r2oh23l16i4z2Zwv43Z33d23t21_2hvd53v23f26i"
    "3i3xXl21f3ouZL6Z1Z1oN8e3giu24AVCKm273a31Cwy26u2u2WfeF3a43T6u4u4Z414oyyI6p41a_c46g3g3_L1y2bV3o43j33z1"
    "GW21Lsxv31KqyuK8Hx_j387se13Z112Yv11FZhRG23W33Z21Uhqq33k3321-2e8313e419-wj2K741NYjl11Rqpg16KKWfeF1X1w"
    "wY13g415tyf2B94Wyvb1391a1_Wi21Jsq21Sz21b1sw13n31IbiY21EgH3f3F03K6Rg1Wpr31o3toj24BD11q3qbT4VCG3a13i26"
    "q1q1l91W1hrj11r3zvP8f1eZu53y23b13S6u1u1exz21m1lG1z3Za11Ds-X27L06i3i3icl28Y4snkH1s1tih19c103g13F16fvy"
    "11Y1xtlI6t4v4ZL9m3_ZYFF33q11w1rja46BBD1k1il11w4_iR1x4-hX31y4sqc23m33j43x11w2k-O3e23V4v1n124-1LC1d1dq"
    "1E6d1d1es11Pj33d32____________F9Z401_3xzt53X11GdWf2353m1363J6x2x2X83d1343k1F13-13y41x3l_16v1-3w51p1s"
    "bcA1g2jx14W4T26x2x2_4I6H1w3ljZ33g26NNZu26NNl_23x23Y33t31c2diw23-31n1mlb2331j1lw11l2laY16p1o1WfeF6u2u"
    "2WfeF1t1yu23p31y3wnf13k41X4wp16y1e1cr66W1W1osH1u3Wme43q36o1o1ZYkC1MebzKF33w33z34p2x116u2u2WfeF1j2bZY"
    "FF33-21Opu13r36w4w4_iR6Di1mcY29u3G3B1f4pek2Aw1m1wA7t10KH71h1bZ23W13N1c1y_11b4__c83y38_1wbj36_2_2nmh4",
    "3d43e3F24b2W246u3u3sZk31X3bj11q4_c2E1o2duZL3X46q4q4_c21Z4dO3z24b3f1C1s4-F1q2gf-21c3yI3o11n2xZy23l26p"
    "2k2_5F03n13c16s1s1ekh1F26PQtD3QAj1m1fI1s2hXg21l4bz21r1l_14X2GG3x33r21h2lzW31m4kw13_31W2hwh43X23Z43g3"
    "3w16n2n2uul27w401r4_p19_2mXt81Y2XcZ4F23j11h4hdp81c4sy16X3l3rr1Kf113n26e1e1qz63f13s46q2q2wsX21n4qy240"
    "1G3v41l1Wuf37u2WWaWWW150A3m21i4Zvu11Z3sB6W4d2WfeF1t2pqa33c33s11v2n13GLGuiL_66_2_2nmh41f2uXs150A1a3xi"
    "M4a4Qa33u41Thhl2Ke103d23w23a31p4yb-CF31o4ucZ71j3qah23E3o43b36j3j3hvg18z1uaW3I7H4Z1KO3r13p41W3WYu11Z2"
    "WyQ8k3ybu23z112Ye23h13Z111qhh23b41e4YWW34g4Q26q4q4uU6UUxfz26b2z4gfZ83W3303f23_219ksj11n3_zz61f3-ua46"
    "2K-g23t23n33_1G13u39201RcB1a1pc2L4YwKX13l13v33T3R9s103C3i11i2il23W2G33f33l36q4q4dL1y2wZp31b1Wc13s33n"
    "43k23c41Sv53K3Y21Lmwn23h4GG1g1zpd13x43i47O01Fsx21t4lO3b11q1kptA3l41m3ivO3F3Y19z201j4mg23Z23m33o33u11"
    "x1yZg27Y4WWaWWW11e2okr46z4z4ofY31Vnya41d3byW4C6k3X1uXaF3L9v1085eo24m1Z12Lo3rwOu2CKi3116gt23q437361Jg"
    "yq26d3d3byW41d1iM3o214qoY44k1s113a4Kd121-1X-11y4WV1v1XA1g2cWq13e43q23i33i2351x2Yx23a13y43S3g49k401Y3"
    "jlZ41t3Zi-1G83I6z4z4xpa69d3WxX8203j23k11-3lpq213ZxZ24p114343Y37V03q14p3c1G381k4sJ1y1pfd41q3weY16y2y2"
    "wZp31w3c11z3_R8-2bu_23e13u26d1r3xj11DuecK1Bgz2I8H3H1t1gYbJ3z43M1W1oeO1Nv-d31y3krY29y3Wkb49j3gpp21d4_"
    "me36Y4Y4WfeF1e3lS3r41X4WZs1Al10gD3g13Z31z2qP3D1o1mU4l2l1O3h34p2O41n1lr1699esR319f208c1txW21PWa25A03-"
    "11Qr16t1t1WfeF6c1c1pe24x3T41r2tqJ9d208_3oqq3Ab30z84X2F21Z4cF3y31g3vv26g2g2iZ71w1zH3y23x11w4o69a406j3"
    "j3hvg16a2a2nD4j1o121X3Y5Lh3gly1X11n2rfs13t31f1yx23d43Y41s4lsU6g3g3nw11v4mc21m2XiN3r31W4-sl13U9X1WceU"
    "3s23s46Z4Z4dI1c3rj13p26c2c2ueF9a2WU3X43e31s1qpkF9LW3C1Gvj13d33v14u41G3j41d2YS3x33w36n2w2Whs11a3Xu23t"
    "11Escv23v43X26l2l2l53u4Aa3m1y43c21o4iZc33_33q31b3qmk11r1xke16b3b3hvf19k104p4FC1z1Y-J1h1fL1Z1do23w41b"
    "4f-v23b3Lu4706v2b2j53Z4Kh11F11W3odc780hZy34f2e181_2g8AL0n13a312ii21t2wp19g2_13h21n3lo21_1onr76c3u3mn"
    "120699esR3c16QQb74l1d1a33X33b41v3xdX41TsY1G0F2333g36W3W3tif63Z13m41RuthE1CuBI9HIAJ3m16p4i1aD1W2XjyC6"
    "z2z2YO1f3_gn21l3hz23d21s3mm23R9h401n4mfY43o16j1j1fd16v3k2ojX41c4eqmF3f41KqunF3w13u31Y2an26j1j1Ys3F31"
    "h4a_f36KKWfeF6c4c4WfeF6_1_1ymc23P3z1Ad40Z26W2W2uYuC6l3x4ou84i4U43p36f3f3aqn21Ojv21b1u13K1l4-83z23j36"
    "w2Fjx-91Y1ac-E6l2Z2-cz11m3ayf13F3k43f21o3e-14u1Z1a33m36f1f1bZ31k3fev21LiX23W36_1q4nZw83O17te13W13z31"
    "6ijt27s1WWaWWW13s33Y13_29u1W16r1o2mqe13r23E4a4Da36j1e4wsb31q2gYwK1i3Wcb36h4h4a_f31i2mvuE15ja26b1a1wv"
    "1Ap30_A3s11y4fd21S-mtK6SSWfeFKi331g4YP9D03k31Iu4377x403C6t2j2sk28k1iR44q1a31Y3zfl6Ku163g29p303g46q2q"
    "2WfeF6a4a4qe11Vsy14q1z1O3Y33518vym13n1Ki211e1clt13o25____________F250A3T1u2pn151XWWWWW19JG3n3343y43e"
    "12WWWWWWWW1F21d1wuk81Hm-c28z4vzu31MujW54r45C6NNv-d31g1Xg23z49q303k16o3Z3na2600bdy33V6HH_ZK5_________"
    "___F23k23m21DkZi13y11h3dot32XWWWWW111aFIBH4-1v1C3Z29g20An30y31y3_hX21y2X_NKg161x1bpd43x2K-141t3bZ169"
    "d4guR3q26e2e2okr45XWWWWW4WWWWWW46o4Y4gkuC1r3hxR3c41Urv13W49i2WsnT1s2stO9MW-WA3b21s4ughH3y23Y24p2r181"
    "X4dupB3i43e2363u26u1e3ueA3q13y36LLcx1303G3t31d3rI9Z201v1oZ24j4y123d41x3tC3s29q20324w3y1G1n2ZzmG6w3w3"
    "eS1t1YZT1v4nbm23n41X2WhqJ5____________F23H3h49n226X1X1XypEK-363j43h11u4wgl21c2cS1_3Xjk71q3uW11w4niWJ"
    "1b3grxH1v2tn11Z4kA688vym12WWWWWWWW1383t46q4q4nZw81a3rle31h2jP3q33p26a3a3rle33S1c3Zs13l13v26-1-1yG3q4"
    "3f36e3c1agA6t1t1_aT6_3X3WfeF1b4Z83A13fhg21g3b31Z1ZjZE1m4xx23J3g13u46t1m1lhw12WWWWWWWW11p4Zd26a4d2gf1"
    "1Rdl11o1lT3X21v3cXx33a33Z11f4-wX34w1l189s4WusY13o11u3bG9o103Y41PlC1z1YsZJ3-3Ax40c21l3eleH6z1z1eB3i13"
    "l32XWWWWW11p3mh23w31Kbwp31z2yR1j3acwA3I1w2jjh13p46n2n2WfeF6Z3Z3pN6p1p1i18l2ed13u33r31FnwY21k4ky16-2-"
    "2WfeF3e41f2-gi47X3XWWWWW1ICH6x1x1ywd43p1G04m3j1a36v4v4_hm23K6c3W3mu63F1_1_tz23k4Ac40_33M1OY36v3v3sow"
    "11W1ydpB3j33-26a1a1WW11z3yhq91s3uE1Y1mO3d2Aj10I1_2ijyE3r43g3B3h36z2r1nu11r2bY21Emmg11j1inK3X41b1vY_8"
    "3l43w13i23W2Az20f81s1th24k34CF117-S1x4tjr16s3s3d13P3c11CWvpIB1t2wn23k31g2tYgG6_2_2WfeF3f26Bg4ku83v43"
    "13c26b1b1Zcl51Y3gnz245_1O1n1Zo16n1n1Zo18o2ix29g4WqH3d19z403_11Tdnu36_2_2_Xe4BC3d33-11n3hV6_2_2iZe414"
    "yv13j21y4ZW11e1xbX33b36o2z4jy250A3h21k1oW19w301o3pro33a11VyY23Z43l21k2fth18m2rI3x33n21y1kXp31Z2eou23"
    "41x2YadJ3Z33L19usj31q2wmlF9f203X33r1G01o4qM1c4Xh2BB3V6OW4kA3v36Tb2lvu33_29j2W3F01y2iqWG3U1Y2wpk19-20",
    "2WWWWWWWW11i4_d11e2ee1676kT3i46z1z1-8Kr219Q_ak46W1W1fw-16QQmE6Y1Y1ag23x13s11u2vu11u1fv11q1p23k25WWWW"
    "WWWWWWWW84F39e30Kx22KW156W3W3yp41y3aafL3W43n1IDH10ppW66RRz61GYF3Q9m2We14t3e113q11d4z3353W16Y3s2_pu83"
    "N3d412Xz23E1n4lpW21HrcM3w43g43z41h4co23e13H1j4gh14h1CG18vjs13e21t4ysaG6x4q3gyg31p2Zd21Sevg31l1_ai23x"
    "23p21v2aYq56g2g2WfeFAj1m1nQ1q42328f3jlW13m11e3t35WWW1XWW1AY2m1uU6W3W3hl13j11_3llx13X16GAhV4JQO3t41g1"
    "yoe17q401u4rck83p36s2s2WfeF1t1tavC303y41a4bo11X2ky23z31a3hso26c4c4ik23b23c46Z2Z2hdK6y1y1kXp31Z1vbl11"
    "o1yse13z16CCqmf2Kt169a328Y4ioaIF36CCntf23o43k13Z11-3XqsD3r21i1nXy31l3kry11w3ul1366m4m4xy23Y13b43q38I"
    "fzQ3m25001p4igY21u3Ywl26f3f3nY83C1r3liu26o1o1fic16l1l1bcm13w33_36g1g1yoe13R1e4Y91p1phwD1Kf_-25XWWWWW"
    "4WWWWWW4Kg216v2c3neq51FgrW41k4yrpG5WWWWWWWWWWWW843Y41MuH3i13q26s2s2o_m46s4s4-bj21j3mN331-2u_g16m3d2-"
    "k41r4tuW33y13l11g3_oz2KD0396t2t2fp26SSixg33a49606j4h3uk51z2dXx33-33p41X4qa23t21l4vD3d2Ky356u3u3Ywl2A"
    "h20uB3c338IEHIFJF29x301w1iE3u1Ku47G13h41i2-46k4k4tnb73y21W2vL3n43j31P_y29Y2-11c1yge2Ao1m1jE3J3p11k3h"
    "v23S7h3XWWWWW11f2qto26u2u2wX23W23n36z2z2pnX32XWWWWW11Bjg21v4pNAw1m1k16s31Y31c2ies36u3u3Ywl21d1dld31_"
    "1Zcq69b1unb46r3r3awm5313D6AAhV1d3tg23g39Q01-1jbv19E03f16z2z2zsX31j2g_11b3bh23l36_1_1qkq63e33c26u2u2m"
    "b63d34o2s116MMq6Kf371h2jok29w301a1pn16-2Z4Yrc43Z23X41l2gE9g301x3m_13u34n2W1414Wx26AAXA1Z3bX21Lvy21X3"
    "xbw1K403a11r1wcf21V_e13b19q401Ofcb33v13V1v3Wj21TzO3r11_2nd13F3h31U-xp317lK6Z4Z4Z-k2G33Z33L3j23W31i4z"
    "tW23h1341x1nkc43e41s1zu29OWmg650A8k2dwq81W4Wjj98n1Wob31QzoO9r106_1q1voq63g2Lg2igY103B45F26t1W1WfeF3f"
    "41Y3cl26m4N_Z39z223Y33l21d4rmcKKi341Et71w4tX13i46l4g4tt19W301z4ekoB3A3Z41e1jB4HP83o33Y26TTzO7K07d4WW"
    "aWWW16X3e2hew16f2f2glf23QC1x4lBIGH351x2gnr13s11p2mrWB3a27-1WWWWWWWWWWWW46c1c1opW23i2Ac1m1tS3r312fXw3"
    "9c401m1ari66X2X2nM3z23x33s26g1j1kvv33d1622pnD1X1pL6W4W4Wjj93W13e11Getm31t4dvXI6W4q4WfeF9l406u4u4ozY7"
    "3q11p3WH7i309GWnX710sbo21y4-YsD1z3anX41b2pl21c4ve23m11z1ucm69o2-11o4d_u31k1wsr2KT21Z1uWd56g4g4tt1F26"
    "t4t4WfeF3_27r409s201r2vP46t1O3e21Y1qha13r23y46k3b4Wz21q3qZZD4m2n1C3z13a31CjZL6b3w3ft41_3wqc23b49h3W3"
    "1Rqu16m2v2xN3f3Av2m1p31Y4usaE3w46h2i1muh73o21q2yYhC13aYZ2Ao4m1gR3v36k4k4zsd43c13t36k2k2hrf11m3eeaIC4"
    "y1O23-1361l1-cY319sz26Z1Z1sdd58a4egmBK343N1j4-Ye23u23x1C3o11-3hT1p4q_l13d43l11t2_ct31d2XkX21c3lc_53q"
    "218oT3C3g41u1xco31h4fgo33o43n2G43G3-3303Y14y222K721n4hm-26Hj3qC1J-D1p1W-q2Kt429v1W33k43y13T399H09i20"
    "3w13w21Snl_33i19Y301W2xc13J1n3sndD1s3-diA41MCKf223z41Dvac29p2V1g3f_gA3y3IHH3t24f1p1G4l3n181e3dK3u43Z"
    "1F23e33s46c4c2Wj21d3nYt2Kv2638Le3ZD-63d36j4j4omw15WWWWWWWWWWWW843n1Ac2m1q31-2nh16IIvYC31Ln1bff2_63Y4"
    "1Z2btZL6u1X4_do36f1f1ZF1u3Xny31a1y-X11b1Xhn13UL1_5-63M1v1mmW28Vgw21r1oz11Fwe15WWW1XWW11h3symD6VZ3YY3"
    "1Ldgo23f26q3q3qZZDF1G21j2ptuB1W3Wv13c3B6FFWk16u3h1rYv714Xcs13X21e4ewZ33q41_1tnd31g2d83-21Bkzp33D3s31"
    "t1tpz26z3z3zoX21f4ol23z36Z2Z2txt39h201m4hj23f49n3W13b23_13B3m33p11Y3iq21l2jiyH4i4d111l4iY21AYi23j33L"
    "3k21Z4qW11o3kj23p23O1Y2dA7h1XWWWWW1331Ktso31X3ne16x2Qgwr1644ka215ag11s1ZS6e4a2tflC622xsD3n41i2kp23y2"
    "6h1h1wWb31r3ta13j46n3n3sndD6Z2Z2mtl71z2cm23l4B1x3zfo33a1Aw3m1oH655eX16RRih-11s2zcxH3r14g1BO3c2F23Z36"
    "Y2Y2pG1d1Wqm27p306v4W1jO3p41e1eft21W4ru-31q1o-Q6Pi3kii23W3C1m1wuf29803K6d2d2XkX21_2jgL6Y2Y2gX13Z21r4"
    "csj23e16n3n3gnX53g24e2j143w33h3IIH1r2Yz19TW31y4ida26h4k3ara71z1_kz23q33r43e2F26EEbB6a4a4roI1a3iky262"
    "b3ytD3z13v16k3b4khfE3b46g1g1s76j1j1jyv36t4t4WfeF6s2s2WfeFG01m2yUF21f3cj23d21w4_b22WWWWWWWW11h2aC1o2Y"
    "J1v3df23i24c1f1O343j11t3ZxwI1-1Zjx23i316dwZG3m43Q4NE81u2lb13I3N3v31x1de23o3F33h1655YH3r36z2z2cm23t14"
    "o1X1O1d4syo13A3w46X1X1c43E3u21l1gjq31q2qE3s16o1o1wg2Ky47Al3m1sN1CeiZ35A06Y3Y3_oA6_2g4qfr23h23a4F1F03"
    "g11o4vi21n2fw29H03Y33z26i4Gg8Kb159f1_yA3b33R1-3djK6l1l1-we76x40hJ3m11Y1fc21HXcyB1k4ns21y1gOF01TsI7x1"
    "XWWWWW119vps61w1oxf2Ai20nD3a23a31w2xf11i1png26k4k4ge13k195F1JhL1z4oo16o2y3qc1666cekA3j26o1t2gj26X1X1"
    "q63b11u4htx13-31Z1jhp32____________FAn40nE1e3x-f21s4sueD1c4zgb118edj93o44d3F83y36TTsI6x3n1ymo33g36d1"
    "1cyn71Y4lip31u1es26W2W2ne19j30381U-_v23n14MKO9v103W471WWWWWWWWWWWW43y46n3n3eraF6y1VmV1f2fW11c3kxj2IJ"
    "HIKJ3c46Y4Y4cmp33w16Z4Z4kZ3G41u3gjwJ4X2x1O9t3Wvrb13m21q4aF6t3t3oZa49t2_l23s44-2B86CCajZ33H3U7006l1l1",
    "jao1LHYtZ5X11DiZ150A6Y1s3kw87CXWWWWW1311z3dta14f4d149FW_26_3b2WfeF1_1-P3-13_11BquO1m3pY19B2ATm1g43l2"
    "6Y2Y2Ze14p1123m33Y26q2q2qE1j3dgz1E1Lwhm43M1k2adZ43s31p2flS3f21OpqZ33C9F_YyR3g413ok21x2beyJ3GK933t26u"
    "4u4wzx13F9R03X11e4cB666bjy61n4pdf33d46W2W2z81y2lXW11j4k9F33c11l4dY21a1pljD9t326Z4Z4cd33Z44r1p1G9n3W1"
    "3b23y21c2ax16W2W2O1Z3skd13l41p3bjv13X21v4vW23j43f36q4p4nJG34Pc1277WWWWWWWWWWWW43e43k417-o11W3z66DDyt"
    "21KWtZ43p2391Z2fU6u1u1es21e1uX-15003X41g2qvaG3B1w3lid31h3jwxH3j36l3l3yHE6x1h4gn_91q3jyXK6-2r4iw26z4z"
    "4mu49Z1V6c3c3Yux43u46u3u3WfeFGW2Ag2m1gE3f18e2iE6z3z3rn73W36l3l3_M3q36n3n3fvaFG312xp11z1atY13l13k21v1"
    "Yh13h46Z3k3dnd13r13s24b45a31d2Zhd12____________F9u3WrmUB1i2cd2ILH14kq_31j1ij-23r41i3yr26a1a1sjW42___"
    "_________F3n46v1m4ii13Y46Z2QgW13z11IzXX11Nsv13w362v3WfeF3i26p3p3hl73u33W13t41o3hXY23v3G21h1j13h11r3x"
    "i21t1fnlG3b43c24A3G1w4wx21Ebru17z401u2ng11s1sy21_2f13c31h2wyd31a4yS3a13n23v26r3r3yB1g1sacE3u26KKxa13"
    "e21Y3q_f21z2vQ1i4vds86VV-Z13j1C6g1g1WfeF1b3nn23z31RXv19_308x4vg23a41m1xXf3F23p11a2pjN6f4f4q13k31a3xf"
    "d3373i46a3a3ijd36Rk1oW23q29x2Wpud16LLsup11o2_WTF03q16Kj2_d13Q3o21o1c11b1g36d3d368-3u_k21o4nzdJ1y3rtd"
    "13V6Lx3WfeF6r3r3Za16b3b3yq21g3hp23A1d1rlzE3t36d3d343o13e13y318gub21n1lS3g23X37TXWWWWW1359n4W31W4ee-2"
    "4y4181y1fqL1c4Y86PPi26x4x4gd26Y3Y3q_f23TAr2_d4_l23l31w1segK1m2qapI1s4fm23x36Z1Z1jhp37c4WWaWWW11Hwmz3"
    "3g36PPY23I6SSi_-76w4UcW310lkeA9V04Y1I83d26Y1Y1_5G23Y381rdd36d1d1fed11_3Zd13W46p3p3vn53n14-1D13s13x43"
    "o31_1gseE1l2iOIMH1m3WWX21Y2fcs26m1m1ndaD3DG21Mhq23y16o4o4WfeF1s3wnqKF03UG31f2s-eB3m33m23E1Czoj73-3F2"
    "1g4XL1GuY16_1_1tyeE1t2horI6m1m1qgp99Q03d34FGC7w101X1gua23X14d4b143a21c1psc83G1Z4mwK3s4G83t26NNif16HH"
    "uqf26f4b2kX26w2w2zi16w2w2qH1y2Wzq54l4K23m14X2F23J1j4XizK3W27H04f3N11q4ZI303d11e4oWo23b11k4dyL3l21p2w"
    "sZ26y49WZy21X4Wg23i31Bxs-13a31j3pZX23P666qke71x1ZY13_33-16x1-2ve43-23y2B3b2F09w301u4_h17b3XWWWWW1393"
    "p38f1wi24W3Q21q3jWS3p26CCzoj738Lm3ea4_6C3l41l1cC331k2xa2F33N3c4611Xtx1Aj4m1zM3Y24h4W245____________F"
    "21Z3_bh21r1YZ2343X21s2djpD6o4o4WfeF1r4s-g31n4_zo71v1umo13k46n3n3kyz99T03j41Y4WhW23S1Z2t_V3_29r1_17f2"
    "WWWWWWWWWWWW41z1zjc36X4X4mj21w3iih33f212bt23m48i2ovx13r31u3oi17f101W1kXY262t4xyA4v3E151XWWWWW150A3j3"
    "Lj3zfO-61h1Zkq33x23h13r43r26v4b4ea83n34c2H436INH3Z29w401z4hhd41c3k-z26OOWjT6t1t1ycw26Fa1k83h33k21n2a"
    "gtE3MKw221v2zr17u306W3W3Z21u2zazI1e2z93q46b3b3-41j1ici7311z3amm21a4rXy23W1Al30iC6Y1Y1oO1p1tv23z43_11"
    "k3Yjv3206v2v2zr117ksu23W33p4Ah2m1gO6h4i4sW13c33d41RxYZ49d307i403Z33Y13C9_103s31q2-Wj26b3b3ZA7j101q1x"
    "kW33Z13q21Km-e31Qnme21o2kyu24V-1G6r1LYd29f403p11Aie13w21t3sjZB3h23g41o1asx3KV36e3e3bfR6v1v1umo13k13o"
    "43AF13a19VW14e1TO3s26z3z3tyl13z1F24y3516z2g2zR6RRrvx29B_11X3adpDLg4pJ_615dZn31TbjXL3Y41l3mgbA3i11x3r"
    "Q1g3xrt31IpO1w4vx16t1t1Wuj13g21d2dd11Y3vWzC6TTWfeF3a44W4r141n1thpFLs2zzb7X16c2c291s1rW2Ke173q13n2G41"
    "x4um23e13Q1o3pz26v3v379k3Wzo73t31DnS1y1xYe23n41UoV1m3et2677tjn51m2jiWK1EbP3X43U1-3fz29Kmzn6354d3841w"
    "1tV6q3q3zqD1X1dZZ91a2ol18Gkfa43y11s4rvf51t2cjt33m2Ax4m1j7K-336v3v3z11f4-XtDF0Kb433W4IOHIPJIQHIRJ0",
    "" };

static const char ALPH[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_";
static int ci;
static const char *cp;
static signed char dec[256];
static unsigned tagc;
static long opno;

static int nextch(void) {
    while (!*cp) cp = CH[++ci];
    return (unsigned char)*cp++;
}
static uint64_t rd(void) {
    uint64_t v = 0;
    int sh = 0;
    for (;;) {
        int d = dec[nextch()];
        v |= (uint64_t)(d & 31) << sh;
        sh += 5;
        if (!(d & 32)) return v;
    }
}
static int64_t rds(void) {
    uint64_t u = rd();
    return (int64_t)(u >> 1) ^ -(int64_t)(u & 1);
}
static void fill(int h) {
    T[h] = (unsigned char)(++tagc * 97u + 13u);
    memset(P[h], T[h], N[h]);
}
static int same(const unsigned char *p, size_t len, unsigned char v) {
    for (size_t i = 0; i < len; i++)
        if (p[i] != v) return 0;
    return 1;
}
static void verify(int h) {
    if (!same(P[h], N[h], T[h])) printf("data corrupted at op %ld\n", opno);
}

int main(void) {
    char stackbuf[64];
    for (int i = 0; i < 64; i++) dec[(unsigned char)ALPH[i]] = (signed char)i;
    cp = CH[0];
    for (;;) {
        opno++;
        int op = (int)rd();
        switch (op) {
        case 0:
            return 0;
        case 1: {
            int h = (int)rd();
            size_t n = (size_t)rd();
            P[h] = my_malloc(n);
            N[h] = n;
            if (!P[h]) { printf("unexpected NULL at op %ld\n", opno); return 0; }
            fill(h);
            break;
        }
        case 2: {
            size_t n = (size_t)rd();
            if (my_malloc(n)) printf("unexpected non-NULL at op %ld\n", opno);
            break;
        }
        case 3: {
            int h = (int)rd();
            verify(h);
            my_free(P[h]);
            break;
        }
        case 4: {
            int h = (int)rd();
            size_t a = (size_t)rd(), b = (size_t)rd();
            P[h] = my_calloc(a, b);
            N[h] = a * b;
            if (!P[h]) { printf("unexpected NULL at op %ld\n", opno); return 0; }
            if (!same(P[h], N[h], 0)) printf("calloc memory not zero at op %ld\n", opno);
            fill(h);
            break;
        }
        case 5: {
            size_t a = (size_t)rd(), b = (size_t)rd();
            if (my_calloc(a, b)) printf("unexpected non-NULL at op %ld\n", opno);
            break;
        }
        case 6: {
            int h = (int)rd(), d = (int)rd();
            size_t n = (size_t)rd();
            verify(h);
            unsigned char old = T[h];
            size_t keep = N[h] < n ? N[h] : n;
            unsigned char *q = my_realloc(P[h], n);
            if (!q) { printf("unexpected NULL at op %ld\n", opno); return 0; }
            if (!same(q, keep, old)) printf("realloc lost data at op %ld\n", opno);
            P[d] = q;
            N[d] = n;
            fill(d);
            break;
        }
        case 7: {
            int h = (int)rd();
            size_t n = (size_t)rd();
            verify(h);
            if (my_realloc(P[h], n)) printf("unexpected non-NULL at op %ld\n", opno);
            break;
        }
        case 8: {
            int h = (int)rd();
            size_t n = (size_t)rd();
            P[h] = my_realloc(NULL, n);
            N[h] = n;
            if (!P[h]) { printf("unexpected NULL at op %ld\n", opno); return 0; }
            fill(h);
            break;
        }
        case 9: {
            int h = (int)rd();
            int64_t delta = rds();
            my_free((void *)((uintptr_t)P[h] + (uintptr_t)delta));
            break;
        }
        case 10: {
            int h = (int)rd();
            int64_t delta = rds();
            size_t n = (size_t)rd();
            if (my_realloc((void *)((uintptr_t)P[h] + (uintptr_t)delta), n))
                printf("unexpected non-NULL at op %ld\n", opno);
            break;
        }
        case 11:
            my_free(NULL);
            break;
        case 12:
            my_free(stackbuf + 8);
            break;
        case 13: {
            size_t n = (size_t)rd();
            if (my_realloc(stackbuf + 8, n)) printf("unexpected non-NULL at op %ld\n", opno);
            break;
        }
        case 14:
            my_free(statbuf + 64);
            break;
        case 15:
            my_set_policy((int)rd());
            break;
        case 16:
            my_set_delay((int)rd());
            break;
        case 17:
            my_heap_dump();
            break;
        case 18:
            printf("-- %lu\n", (unsigned long)rd());
            break;
        case 19:
            my_leak_report();
            break;
        case 20: {
            int h = (int)rd();
            int j = (int)rd();
            P[h][N[h] + (size_t)j] = (unsigned char)(0x11 + j);
            break;
        }
        case 21: {
            int h = (int)rd();
            size_t j = (size_t)rd();
            P[h][j] = (unsigned char)rd();
            break;
        }
        default:
            printf("bad op\n");
            return 1;
        }
    }
}
