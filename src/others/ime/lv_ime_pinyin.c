/**
 * @file lv_ime_pinyin.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_ime_pinyin.h"
#if LV_USE_IME_PINYIN != 0

#include <stdio.h>

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS    &lv_ime_pinyin_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_ime_pinyin_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_ime_pinyin_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_ime_pinyin_style_change_event(lv_event_t * e);
static void lv_ime_pinyin_kb_event(lv_event_t * e);
static void lv_ime_pinyin_cand_panel_event(lv_event_t * e);

static void init_pinyin_dict(lv_obj_t * obj, lv_pinyin_dict_t * dict);
static void pinyin_input_proc(lv_obj_t * obj);
static void pinyin_page_proc(lv_obj_t * obj, uint16_t btn);
static char * pinyin_search_matching(lv_obj_t * obj, char * py_str, uint16_t * cand_num);
static void pinyin_ime_clear_data(lv_obj_t * obj);

#if LV_IME_PINYIN_USE_K9_MODE
    static void pinyin_k9_init_data(lv_obj_t * obj);
    static void pinyin_k9_get_legal_py(lv_obj_t * obj, char * k9_input, const char * py9_map[]);
    static bool pinyin_k9_is_valid_py(lv_obj_t * obj, char * py_str);
    static void pinyin_k9_fill_cand(lv_obj_t * obj);
    static void pinyin_k9_cand_page_proc(lv_obj_t * obj, uint16_t dir);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lv_ime_pinyin_class = {
    .constructor_cb = lv_ime_pinyin_constructor,
    .destructor_cb  = lv_ime_pinyin_destructor,
    .width_def      = LV_SIZE_CONTENT,
    .height_def     = LV_SIZE_CONTENT,
    .group_def      = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size  = sizeof(lv_ime_pinyin_t),
    .base_class     = &lv_obj_class
};

#if LV_IME_PINYIN_USE_K9_MODE
static char * lv_btnm_def_pinyin_k9_map[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 21] = {\
                                                                                ",\0", "123\0",  "abc \0", "def\0",  LV_SYMBOL_BACKSPACE"\0", "\n\0",
                                                                                ".\0", "ghi\0", "jkl\0", "mno\0",  LV_SYMBOL_KEYBOARD"\0", "\n\0",
                                                                                "?\0", "pqrs\0", "tuv\0", "wxyz\0",  LV_SYMBOL_NEW_LINE"\0", "\n\0",
                                                                                LV_SYMBOL_LEFT"\0", "\0"
                                                                               };

static lv_btnmatrix_ctrl_t default_kb_ctrl_k9_map[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 17] = { 1 };
static char   lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 2][LV_IME_PINYIN_K9_MAX_INPUT] = {0};
#endif

static char   lv_pinyin_cand_str[LV_IME_PINYIN_CAND_TEXT_NUM][4];
static char * lv_btnm_def_pinyin_sel_map[LV_IME_PINYIN_CAND_TEXT_NUM + 3];

#if LV_IME_PINYIN_USE_DEFAULT_DICT
lv_pinyin_dict_t lv_ime_pinyin_def_dict[] = {
    { "a", "阿啊呵啊呵阿啊呵腌吖锕啊呵嗄" },
    { "ai", "呆挨癌皑捱爱碍艾唉哎隘暧嗳瑷嗌嫒砹哀挨埃唉哎捱锿矮哎蔼霭嗳" },
    { "an", "俺铵揞埯安谙鞍氨庵桉鹌广厂案按暗岸黯胺犴" },
    { "ang", "昂肮盎仰" },
    { "ao", "熬凹熬敖嚣嗷鏖鳌翱獒聱螯廒遨奥澳傲懊坳拗骜岙鏊袄拗媪" },
    { "ba", "把爸罢霸坝耙灞鲅吧罢八拔跋茇菝魃八吧巴叭芭扒疤笆粑岜捌把靶钯" },
    { "bai", "掰百摆伯柏佰捭败拜呗稗白" },
    { "ban", "般班搬斑颁扳瘢癍办半伴扮瓣拌绊版板阪坂钣舨" },
    { "bang", "棒膀傍磅谤镑蚌蒡帮邦浜梆膀榜绑" },
    { "bao", "薄雹保宝饱堡葆褓鸨报暴抱爆鲍曝刨瀑豹趵包胞炮剥褒苞孢煲龅" },
    { "bei", "被备背辈倍贝蓓惫悖狈焙邶钡孛碚褙鐾鞴臂呗北背悲杯碑卑陂埤萆鹎" },
    { "ben", "奔贲锛本苯畚奔笨夯坌" },
    { "beng", "甭崩绷嘣绷蹦迸甏泵蚌绷" },
    { "bi", "必毕币秘避闭壁臂弊辟碧拂毙蔽庇璧敝泌陛弼篦婢愎痹铋裨濞髀庳毖滗蓖埤芘嬖荜贲畀萆薜筚箅哔襞跸狴逼比笔彼鄙匕俾妣吡秕舭鼻荸" },
    { "bian", "贬扁匾碥窆褊编边鞭砭煸蝙笾鳊边便变遍辩辨辫卞苄汴忭弁缏" },
    { "biao", "表裱婊标彪勺镖膘骠镳杓飚飑飙瘭髟鳔" },
    { "bie", "憋瘪鳖别瘪别蹩" },
    { "bin", "宾滨彬斌缤濒槟傧玢豳镔鬓殡摈膑髌" },
    { "bing", "并兵冰槟并病摒饼屏丙柄秉炳禀邴" },
    { "bo", "般波播拨剥玻饽菠钵趵簸跛百博伯勃薄泊柏驳魄脖搏膊舶礴帛铂箔渤钹孛亳鹁踣薄柏簸掰擘檗卜啵" },
    { "bu", "不部布步怖簿埔埠瓿钚不醭补捕堡卜哺卟逋晡钸" },
    { "ca", "礤擦拆嚓" },
    { "cai", "采彩踩睬猜采菜蔡才财材裁" },
    { "can", "残惭蚕参餐骖惨灿掺璨孱粲惨黪" },
    { "cang", "藏苍仓沧舱伧" },
    { "cao", "草操糙曹槽嘈漕螬艚" },
    { "ce", "策册测厕侧" },
    { "cen", "岑涔参" },
    { "ceng", "蹭噌曾层" },
    { "cha", "差插叉碴喳嚓杈馇锸差刹叉诧岔衩杈汊姹查察茶叉茬碴楂猹搽槎檫叉衩镲" },
    { "chai", "差拆钗虿瘥柴豺侪" },
    { "chan", "产铲阐谄冁蒇骣单缠禅蝉馋潺蟾婵谗廛孱镡澶躔颤忏羼搀掺觇" },
    { "chang", "长场常尝肠偿倘裳嫦徜苌场厂敞氅昶惝昌娼猖伥阊菖鲳唱畅倡怅鬯" },
    { "chao", "超抄吵钞绰剿焯怊炒吵朝潮嘲巢晁耖" },
    { "che", "彻撤澈掣坼尺扯车砗" },
    { "chen", "郴臣辰尘晨忱沉陈趁衬橙沈称秤" },
    { "cheng", "逞骋裎称撑秤瞠噌铛柽蛏成城程承诚盛乘呈惩澄橙丞埕枨塍铖裎酲称秤" },
    { "chi", "尺齿耻侈褫豉赤斥翅啻炽敕叱饬傺彳瘛吃痴哧嗤蚩笞鸱媸螭眵魑持迟池驰匙弛踟墀茌篪坻" },
    { "chong", "种重崇虫冲充涌憧忡艟舂茺宠冲铳" },
    { "chou", "愁仇筹酬绸踌惆畴稠帱俦雠臭抽瘳丑瞅" },
    { "chu", "处触畜矗怵搐绌黜亍憷出初樗除厨躇橱雏锄蜍刍滁蹰处楚储础杵褚楮" },
    { "chuai", "揣搋揣啜踹嘬膪揣" },
    { "chuan", "传船遄椽舡喘舛穿川巛氚串钏" },
    { "chuang", "闯创窗疮床幢创怆" },
    { "chui", "吹炊垂锤捶陲椎槌棰" },
    { "chun", "春椿蝽纯唇醇淳鹑莼蠢" },
    { "chuo", "戳踔绰啜辍龊" },
    { "ci", "词辞慈磁瓷兹茨雌祠茈鹚糍此差刺疵呲次刺赐伺" },
    { "cong", "从匆聪葱囱苁骢璁枞从丛琮淙" },
    { "cou", "凑楱辏腠" },
    { "cu", "促簇醋卒猝蹴蹙蔟酢徂殂粗" },
    { "cuan", "窜篡爨蹿撺汆镩攒" },
    { "cui", "璀衰催摧崔隹榱脆粹萃翠瘁悴淬毳啐" },
    { "cun", "忖存蹲村皴寸" },
    { "cuo", "撮脞搓撮磋蹉嵯矬痤瘥鹾错措挫厝锉" },
    { "da", "答搭嗒耷褡哒大打达答瘩沓鞑怛笪靼妲打塔疸" },
    { "dai", "待呆呔逮歹傣大代带待戴袋贷逮殆黛怠玳岱迨骀绐埭甙" },
    { "dan", "但担石弹淡旦蛋诞惮啖澹氮萏瘅担胆掸赕疸瘅单担丹耽眈殚箪儋瘅聃郸" },
    { "dang", "当裆铛党挡谠当荡档挡宕菪凼砀" },
    { "dao", "到道倒悼盗稻焘帱纛刀叨忉氘导倒岛蹈捣祷叨" },
    { "de", "的地得底得德锝" },
    { "dei", "得" },
    { "deng", "登灯蹬噔簦邓凳瞪澄蹬磴镫嶝等戥" },
    { "di", "底抵诋邸砥坻柢氐骶的敌迪笛涤嘀狄嫡翟荻籴觌镝提低滴堤嘀氐镝羝的地第帝弟递蒂缔谛睇棣娣碲绨" },
    { "dia", "嗲" },
    { "dian", "颠滇掂癫巅电店甸淀垫殿奠惦佃玷簟坫靛钿癜阽点典碘踮丶" },
    { "diao", "雕刁凋叼貂碉鲷调掉吊钓铫铞鸟" },
    { "die", "爹跌踮叠迭碟谍蝶喋佚牒耋蹀堞瓞揲垤鲽" },
    { "ding", "定订钉铤腚锭碇啶丁盯钉叮町酊疔仃耵玎顶鼎酊" },
    { "diu", "丢铥" },
    { "dong", "动洞冻栋恫侗垌峒胨胴硐东冬咚岽氡鸫懂董硐" },
    { "dou", "斗抖陡蚪都兜蔸篼读斗豆逗窦痘" },
    { "du", "度渡肚杜妒镀芏蠹读独顿毒渎牍犊黩髑椟都督嘟肚睹堵赌笃" },
    { "duan", "短断段锻缎煅椴簖端" },
    { "dui", "对队兑敦碓憝怼镦堆" },
    { "dun", "顿盾钝炖遁沌囤砘盹趸吨敦蹲墩礅镦" },
    { "duo", "朵躲垛哚缍度夺踱铎多咄哆掇裰舵堕跺剁惰垛驮沲柁" },
    { "e", "恶饿扼愕遏噩呃厄鄂轭颚鳄谔锷萼腭垩鹗苊阏阿婀屙呃恶额俄哦鹅娥峨蛾讹莪锇" },
    { "ei", "诶" },
    { "en", "恩蒽摁" },
    { "er", "二贰佴尔耳迩饵洱珥铒而儿鸸鲕" },
    { "fa", "发罚乏伐阀筏垡法砝发珐" },
    { "fan", "反返翻番帆藩幡蕃饭犯范贩泛梵畈凡烦繁泛樊蕃燔矾蘩钒蹯" },
    { "fang", "放方芳妨坊邡枋钫访仿纺彷舫房防妨坊肪鲂" },
    { "fei", "菲匪诽斐蜚翡悱篚榧非飞啡菲扉霏妃绯蜚鲱费废沸肺吠痱狒镄芾肥腓淝" },
    { "fen", "粉坟焚汾棼鼢分纷氛芬吩酚玢分份奋愤粪忿偾瀵鲼" },
    { "feng", "奉缝凤俸葑风封丰峰疯锋蜂枫烽酆葑沣砜逢缝冯讽唪" },
    { "fo", "佛" },
    { "fou", "否缶" },
    { "fu", "府父腐抚辅甫俯斧脯釜腑拊滏黼服复父负副富付妇附赴腹覆赋傅缚咐阜讣驸赙馥蝮鲋鳆咐夫肤敷孵呋稃麸趺跗夫服福佛幅伏符浮扶弗拂袱俘芙孚匐辐涪氟桴蜉苻茯莩菔幞怫艴郛绂绋凫祓砩黻罘稃蚨芾蝠" },
    { "ga", "夹咖嘎胳伽旮嘎噶轧尜钆嘎尕尬" },
    { "gai", "改该赅垓陔概盖丐钙芥溉戤" },
    { "gan", "干甘肝杆尴乾竿坩苷柑泔矸疳酐感敢赶杆橄秆擀澉干赣淦绀旰" },
    { "gang", "钢杠戆筻刚钢纲缸扛杠冈肛罡港岗" },
    { "gao", "告膏诰郜锆稿搞藁槁缟镐杲高糕膏皋羔睾篙槔" },
    { "ge", "个各铬硌虼歌格哥戈割胳搁疙咯鸽屹仡圪纥袼个各合盖葛哿舸革格隔葛阁胳搁蛤嗝骼颌搿膈镉塥鬲" },
    { "gei", "给" },
    { "gen", "亘艮茛根跟艮哏" },
    { "geng", "更耿颈梗哽鲠埂绠更耕庚羹赓" },
    { "gong", "工公共红供功攻宫恭躬龚弓肱蚣觥巩拱汞珙共供贡" },
    { "gou", "够购构勾觏垢诟媾遘彀狗苟岣枸笱句沟勾钩篝佝枸缑鞲" },
    { "gu", "骨古股鼓骨谷贾汩蛊毂鹄牯臌诂瞽罟钴嘏蛄鹘故顾固估雇锢梏牿崮痼鲴姑骨孤估辜咕呱箍沽菇轱鸪毂菰蛄酤觚" },
    { "gua", "寡呱剐挂褂卦诖括瓜刮呱栝胍鸹" },
    { "guai", "乖掴怪拐" },
    { "guan", "观惯冠贯罐灌掼盥涫鹳管馆莞关观官冠棺矜莞倌纶鳏" },
    { "guang", "广犷逛桄光咣胱桄" },
    { "gui", "鬼轨诡癸匦庋宄晷簋规归瑰龟硅闺皈傀圭妫鲑贵桂跪柜刽炔刿桧炅鳜" },
    { "gun", "棍滚鲧衮绲磙辊" },
    { "guo", "过锅郭涡聒蝈崞埚呙国帼掴馘虢果裹猓椁蜾过" },
    { "ha", "哈虾蛤哈铪" },
    { "hai", "嘿咳嗨还孩骸害亥骇氦海胲醢" },
    { "han", "酣憨顸鼾蚶喊罕阚汉汗憾翰撼旱捍悍瀚焊颔菡撖含寒汗韩涵函晗焓邯邗" },
    { "hang", "夯行巷沆行航吭杭绗珩颃" },
    { "hao", "蒿薅嚆好号浩耗皓昊灏镐颢号毫豪嚎壕貉嗥濠蚝好郝" },
    { "he", "和何喝赫吓贺荷鹤壑褐喝呵诃嗬和何合河核盒禾荷阂涸阖貉曷颌劾菏盍纥蚵翮" },
    { "hei", "黑嘿嗨" },
    { "hen", "痕恨很狠" },
    { "heng", "哼亨横行横衡恒蘅珩桁" },
    { "hong", "哄讧蕻哄轰哄烘薨訇红洪鸿宏虹弘泓闳蕻黉荭" },
    { "hou", "侯喉猴瘊篌糇骺后候後厚侯逅堠鲎吼" },
    { "hu", "虎浒唬琥和胡湖糊核壶狐葫弧蝴囫瑚斛鹄醐猢槲鹕觳煳鹘护户互糊虎沪祜扈戽笏岵怙瓠鹱冱乎呼戏忽糊惚唿滹轷烀" },
    { "hua", "话华化划画桦华化花哗砉华划滑哗豁猾骅铧" },
    { "huai", "坏划怀徊淮槐踝" },
    { "huan", "欢獾还环寰鬟桓圜洹郇缳锾萑缓换患幻唤宦焕痪涣浣奂擐豢漶逭鲩" },
    { "huang", "晃恍谎幌黄皇煌惶徨璜簧凰潢蝗蟥遑隍磺癀湟篁鳇晃荒慌肓" },
    { "hui", "挥辉灰恢徽堕诙晖麾珲咴虺隳毁悔虺会汇惠慧溃绘讳贿晦秽诲彗烩荟卉喙恚浍哕缋桧蕙蟪回徊蛔茴洄" },
    { "hun", "混魂浑馄珲混诨溷婚昏荤阍" },
    { "huo", "和活豁劐攉锪耠和或获货祸惑霍豁藿嚯镬蠖火伙夥钬" },
    { "i", "" },
    { "ji", "其几期机基击奇激积鸡迹绩饥缉圾姬矶肌讥叽稽畸跻羁嵇唧畿齑箕屐剞玑赍犄墼芨丌咭笄乩革及即辑级极集急籍吉疾嫉藉脊棘汲岌笈瘠诘亟楫蒺殛佶戢嵴蕺记系计济寄际技纪继既齐季寂祭忌剂冀妓骥蓟悸伎暨霁稷偈鲫髻觊荠跽哜鲚洎芰几给己革济纪挤脊戟虮掎麂" },
    { "jia", "夹颊戛荚郏恝铗袷蛱家加佳夹嘉茄挟枷珈迦伽浃痂笳葭镓袈跏价假架驾嫁稼假角脚甲搅贾缴绞饺矫佼狡剿侥皎胛铰挢岬徼湫敫钾嘏瘕家" },
    { "jian", "简减检剪捡拣俭碱茧柬蹇謇硷睑锏枧戬谫囝裥笕翦趼间坚监渐兼艰肩浅尖奸溅煎歼缄笺菅蒹搛湔缣戋犍鹣鲣鞯见间件建监渐健剑键荐鉴践舰箭贱溅槛谏僭涧饯毽锏楗腱牮踺" },
    { "jiang", "将强降酱浆虹匠犟绛洚糨讲奖蒋桨耩将江疆姜浆僵缰茳礓豇" },
    { "jiao", "教交焦骄郊胶椒娇浇姣跤蕉礁鲛僬鹪蛟艽茭角脚搅缴绞饺矫佼狡剿侥皎挢徼湫敫铰教觉校叫较轿嚼窖酵噍峤徼醮嚼矫峤" },
    { "jie", "解姐接结节街阶皆揭楷嗟秸疖喈界解价介借戒届藉诫芥疥蚧骱结节杰捷截洁劫竭睫桔拮孑诘桀碣偈颉讦婕羯鲒家价" },
    { "jin", "今金禁津斤筋巾襟矜衿进近尽仅禁劲晋浸靳缙烬噤觐荩赆妗尽仅紧谨锦瑾馑卺廑堇槿" },
    { "jing", "经京精惊睛晶荆兢鲸泾旌茎腈菁粳经境竟静敬镜劲竞净径靖痉迳胫弪婧獍靓警景井颈憬阱儆刭肼" },
    { "jiong", "扃窘炯迥炅" },
    { "jiu", "九酒久韭灸玖就旧救疚舅咎臼鹫僦厩桕柩究纠揪鸠赳啾阄鬏" },
    { "ju", "据句具剧巨聚拒距俱惧沮瞿锯炬趄飓踞遽倨钜犋屦榘窭讵醵苣局菊桔橘锔车据且居俱拘驹鞠锯趄掬疽裾苴椐锔狙琚雎鞫举柜矩咀沮踽龃榉莒枸" },
    { "juan", "卷锩圈卷俊倦眷隽绢狷桊鄄捐圈娟鹃涓镌蠲" },
    { "jue", "嗟撅噘倔觉绝决角脚嚼掘诀崛爵抉倔獗厥蹶攫谲矍孓橛噱珏桷劂爝镢蕨觖蹶" },
    { "jun", "军均君钧筠龟菌皲麇俊峻隽菌郡骏竣捃浚" },
    { "ka", "卡咯咔佧胩咖喀咔" },
    { "kai", "慨凯铠楷恺蒈剀垲锴忾开揩锎" },
    { "kan", "侃砍坎槛阚莰看刊堪勘龛戡看嵌瞰阚" },
    { "kang", "抗炕亢伉闶钪扛康慷糠闶" },
    { "kao", "靠铐犒考烤拷栲尻" },
    { "ke", "科颗柯呵棵苛磕坷嗑瞌轲稞疴蝌钶窠颏珂髁可渴坷轲岢咳壳颏可克客刻课恪嗑溘骒缂氪锞蚵" },
    { "ken", "裉肯恳啃垦龈" },
    { "keng", "坑吭铿" },
    { "kong", "空控空倥崆箜恐孔倥" },
    { "kou", "扣寇叩蔻筘口抠芤眍" },
    { "ku", "哭枯窟骷刳堀苦库裤酷喾绔" },
    { "kua", "跨挎胯垮侉夸" },
    { "kuai", "会快块筷脍哙侩狯浍郐蒯" },
    { "kuan", "款宽髋" },
    { "kuang", "夼框筐匡哐诓况矿框旷眶邝圹纩贶狂诳" },
    { "kui", "傀跬亏窥盔岿悝愧溃馈匮喟聩篑蒉愦魁睽逵葵奎馗夔喹隗暌揆蝰" },
    { "kun", "困昆坤鲲锟醌琨髡捆悃阃" },
    { "kuo", "括适阔扩廓栝蛞" },
    { "la", "落拉辣腊蜡剌瘌拉喇拉啦喇垃邋蓝啦拉喇旯砬" },
    { "lai", "来莱徕涞崃铼赖睐癞籁赉濑" },
    { "lan", "兰蓝栏拦篮澜婪岚斓阑褴镧谰烂滥懒览揽榄缆漤罱" },
    { "lang", "浪郎莨蒗阆啷狼郎廊琅螂榔锒稂阆朗" },
    { "lao", "捞劳牢唠崂铹痨醪落络唠烙酪涝耢老姥佬潦栳铑" },
    { "le", "肋了乐勒仂叻泐鳓" },
    { "lei", "累蕾垒磊儡诔耒嘞类泪累擂肋酹勒擂累雷擂羸镭嫘缧檑" },
    { "leng", "冷楞棱塄棱愣" },
    { "li", "离丽黎璃漓狸梨篱犁厘罹藜骊蜊黧缡喱鹂嫠蠡鲡蓠璃哩里理李礼哩鲤俚逦娌悝澧锂蠡醴鳢哩力利立历例丽励厉莉笠粒俐栗隶吏沥雳莅戾俪砺痢郦詈荔枥呖唳猁溧砾栎轹傈坜苈疠疬蛎鬲篥粝跞藓" },
    { "lia", "俩" },
    { "lian", "练恋炼链殓楝潋联连怜莲廉帘涟镰裢濂臁奁蠊鲢脸敛琏蔹裣" },
    { "liang", "量良梁凉粮粱踉莨椋墚量亮辆凉谅晾踉靓两俩魉" },
    { "liao", "撩撂了料廖镣撩撂尥钌了潦燎蓼钌聊疗辽僚寥撩撂缭寮燎嘹獠鹩" },
    { "lie", "列烈裂劣猎趔冽洌捩埒躐鬣裂咧咧" },
    { "lin", "林临秘邻琳淋霖麟鳞磷嶙辚粼遴啉瞵凛懔檩廪淋吝躏赁蔺膦" },
    { "ling", "领令岭拎令灵零龄凌玲铃陵伶聆囹棱菱苓翎棂瓴绫酃泠羚蛉柃鲮令另呤" },
    { "liu", "柳绺锍六陆溜碌遛馏镏鹨留流刘瘤榴浏硫琉遛馏镏旒骝鎏溜熘" },
    { "lo", "咯" },
    { "long", "笼拢垄陇垅隆龙隆笼胧咙聋珑窿茏栊泷砻癃弄" },
    { "lou", "搂篓嵝楼喽偻娄髅蝼蒌耧喽露陋漏镂瘘搂" },
    { "lu", "六路陆录露绿鹿碌禄辘麓赂漉戮簏鹭潞璐辂渌蓼逯鲁芦卤虏掳橹镥卢炉庐芦颅泸轳鲈垆胪鸬舻栌噜撸轳氇" },
    { "lv", "旅履屡侣缕吕捋铝偻褛膂稆律绿率虑滤氯驴榈闾" },
    { "luan", "卵乱峦挛孪栾銮滦鸾娈脔" },
    { "lue:", "略掠锊掠" },
    { "lun", "论轮伦沦仑抡囵纶论抡" },
    { "luo", "裸倮蠃瘰罗逻萝螺锣箩骡猡椤脶镙罗落络洛骆咯摞烙珞泺漯荦硌雒落罗捋" },
    { "ma", "吗麻蟆么吗嘛妈麻摩抹蚂嬷骂蚂唛杩马吗码玛蚂犸" },
    { "mai", "买荬埋霾卖麦迈脉劢" },
    { "man", "满螨埋蛮馒瞒蔓谩鳗鞔慢漫曼蔓谩墁幔缦熳镘颟" },
    { "mang", "莽蟒漭忙茫盲芒氓邙硭" },
    { "mao", "冒贸帽貌茂耄瑁懋袤瞀猫毛猫矛茅髦锚牦旄蝥蟊茆卯铆峁泖昴" },
    { "me", "么麽" },
    { "mei", "妹魅昧谜媚寐袂美每镁浼没眉梅媒枚煤霉玫糜酶莓嵋湄楣猸镅鹛" },
    { "men", "们闷懑焖门扪钔闷" },
    { "meng", "蒙盟朦氓萌檬瞢甍礞虻艨蒙梦孟蒙猛勐懵蠓蜢锰艋" },
    { "mi", "眯咪迷弥谜靡糜醚麋猕祢縻蘼米眯靡弭敉脒芈密秘觅蜜谧泌汨宓幂嘧糸" },
    { "mian", "棉眠绵免缅勉腼冕娩渑湎沔眄黾面" },
    { "miao", "妙庙缪喵描苗瞄鹋秒渺藐缈淼杪邈眇" },
    { "mie", "灭蔑篾蠛乜咩" },
    { "min", "民珉岷缗玟苠敏悯闽泯皿抿闵愍黾鳘" },
    { "ming", "命名明鸣盟铭冥茗溟瞑暝螟酩" },
    { "miu", "谬缪" },
    { "mo", "没万默莫末冒磨寞漠墨抹陌脉嘿沫蓦茉貉秣镆殁瘼耱貊貘无模麽磨摸摩魔膜蘑馍摹谟嫫摸抹" },
    { "mou", "某谋牟眸缪鍪蛑侔哞" },
    { "mu", "模毪目木幕慕牧墓募暮牟穆睦沐坶苜仫钼母姆姥亩拇牡" },
    { "na", "那南哪呐拿镎那哪那呢纳娜呐捺钠肭衲" },
    { "nai", "奈耐鼐佴萘柰哪乃奶氖艿" },
    { "nan", "难囝囡腩蝻赧难南男楠喃" },
    { "nang", "囊囔囊馕馕攮曩" },
    { "nao", "努挠呶猱铙硇蛲闹淖孬脑恼瑙垴" },
    { "ne", "呢呐讷哪呢呐" },
    { "nei", "那内哪馁" },
    { "nen", "嫩恁" },
    { "neng", "能" },
    { "ng", "嗯唔嗯" },
    { "ni", "妮你拟旎祢呢尼泥倪霓坭猊怩铌鲵泥尿逆匿腻昵溺睨慝伲" },
    { "nian", "年粘黏鲇鲶蔫拈念廿酿埝碾捻撵辇" },
    { "niang", "酿娘酿" },
    { "niao", "鸟袅嬲茑尿溺脲" },
    { "nie", "涅聂孽蹑嗫啮镊镍乜陧颞臬蘖捏" },
    { "nin", "您恁" },
    { "ning", "宁凝拧咛狞柠苎甯聍宁拧泞佞拧" },
    { "niu", "拗纽扭钮狃忸妞牛" },
    { "nong", "农浓侬哝脓弄" },
    { "nou", "耨" },
    { "nu", "奴孥驽努弩胬怒" },
    { "nv", "恧衄女钕" },
    { "nuan", "暖" },
    { "nue:", "虐疟" },
    { "nuo", "诺懦糯喏搦锘娜挪傩" },
    { "o", "哦噢喔" },
    { "ou", "偶呕藕耦呕沤怄区欧殴鸥讴瓯沤" },
    { "pa", "爬扒耙杷钯筢派扒趴啪葩琶怕帕" },
    { "pai", "派湃蒎哌排迫排牌徘俳拍" },
    { "pan", "般盘胖磐蹒爿蟠判盼叛畔拚襻袢泮番攀潘扳" },
    { "pang", "胖耪乓膀滂旁庞膀磅彷螃逄" },
    { "pao", "炮抛泡脬跑炮袍刨咆狍匏庖跑炮泡疱" },
    { "pei", "配佩沛辔帔旆霈呸胚醅陪培赔裴锫" },
    { "pen", "盆湓喷" },
    { "peng", "烹抨砰澎怦嘭捧朋鹏彭棚蓬膨篷澎硼堋蟛碰" },
    { "pi", "否匹劈痞癖圮擗吡庀仳疋皮疲啤脾琵毗郫鼙裨埤陴芘枇罴铍陂蚍蜱貔屁辟僻譬媲淠甓睥批坏披辟劈坯霹噼丕纰砒邳铍" },
    { "pian", "片篇偏翩扁犏便蹁缏胼骈谝片骗" },
    { "piao", "漂瞟缥殍莩漂飘剽缥螵票漂骠嘌朴瓢嫖" },
    { "pie", "撇瞥氕撇丿苤" },
    { "pin", "品榀聘牝拼拚姘贫频苹嫔颦" },
    { "ping", "乒娉俜平评瓶凭萍屏冯苹坪枰鲆" },
    { "po", "破迫朴魄粕珀繁婆鄱皤颇坡泊朴泼陂泺攴钋叵钷笸" },
    { "pou", "掊裒掊剖" },
    { "pu", "葡蒲仆脯菩匍璞濮莆镤普堡朴谱浦溥埔圃氆镨蹼铺扑仆噗暴铺堡曝瀑" },
    { "qi", "期七妻欺缉戚凄漆栖沏蹊嘁萋槭柒欹桤其奇棋齐旗骑歧琪祈脐祺祁崎琦淇岐荠俟耆芪颀圻骐畦亓萁蕲畦蛴蜞綦鳍麒气妻器汽齐弃泣契迄砌憩汔亟讫葺碛起企启岂乞稽绮杞芑屺綮" },
    { "qia", "卡恰洽髂掐伽葜袷" },
    { "qian", "浅遣谴缱肷千签牵迁谦铅骞悭芊愆阡仟岍扦佥搴褰钎欠歉纤嵌倩堑茜芡慊椠前钱潜乾虔钳掮黔荨钤犍箝鬈" },
    { "qiang", "将枪抢腔呛锵跄羌戕戗镪蜣锖呛跄炝戗强抢襁镪羟强墙蔷樯嫱" },
    { "qiao", "桥乔侨瞧翘蕉憔樵峤谯荞鞒翘俏窍壳峭撬鞘诮谯悄敲雀锹跷橇缲硗劁悄巧雀愀" },
    { "qie", "茄伽切且切窃怯趄妾砌惬锲挈郄箧慊" },
    { "qin", "沁揿吣寝琴秦勤芹擒矜覃禽噙廑溱檎锓嗪芩螓亲钦侵衾" },
    { "qing", "情晴擎氰檠黥青清轻倾卿氢蜻圊鲭亲庆罄磬箐綮请顷謦苘" },
    { "qiong", "穷琼穹茕邛蛩筇跫銎" },
    { "qiu", "秋邱丘龟蚯鳅楸湫糗求球仇囚酋裘虬俅遒赇泅逑犰蝤巯鼽" },
    { "qu", "去趣觑阒区曲屈趋驱躯觑岖蛐祛蛆麴诎黢取曲娶龋苣戌渠瞿衢癯劬璩氍朐磲鸲蕖蠼蘧" },
    { "quan", "圈悛劝券犬绻畎全权泉拳诠颧蜷荃铨痊醛辁筌鬈" },
    { "que", "瘸却确雀榷鹊阕阙悫缺阙炔" },
    { "qun", "群裙麇逡" },
    { "ran", "然燃髯蚺染冉苒" },
    { "rang", "让瓤禳穰嚷攘壤禳嚷" },
    { "rao", "扰绕娆绕饶娆桡荛" },
    { "re", "若惹喏热" },
    { "ren", "任认韧刃纫饪仞葚妊轫衽忍稔荏人任仁壬" },
    { "reng", "仍扔" },
    { "ri", "日" },
    { "rong", "冗容荣融蓉溶绒熔榕戎嵘茸狨肜蝾" },
    { "rou", "柔揉蹂糅鞣肉" },
    { "ru", "辱乳汝入褥缛洳溽蓐如儒茹嚅濡孺蠕薷铷襦颥" },
    { "ruan", "软阮朊" },
    { "rui", "瑞锐芮睿枘蚋蕊蕤" },
    { "run", "润闰" },
    { "ruo", "若弱偌箬" },
    { "sa", "萨卅飒脎撒仨挲洒撒" },
    { "sai", "思塞腮鳃噻赛塞" },
    { "san", "散伞馓糁霰三叁毵散" },
    { "sang", "丧丧桑嗓搡磉颡" },
    { "sao", "扫嫂骚搔臊缲缫鳋扫梢臊埽瘙" },
    { "se", "色塞涩瑟啬铯穑" },
    { "sen", "森" },
    { "seng", "僧" },
    { "sha", "沙啥厦煞霎嗄歃唼傻杀沙刹纱杉莎煞砂挲鲨痧裟铩" },
    { "shai", "色晒筛酾" },
    { "shan", "山衫删煽扇珊杉栅跚姗潸膻芟埏钐舢苫髟闪陕掺掸单善扇禅擅膳讪汕赡缮嬗掸骟剡苫鄯钐疝蟮鳝" },
    { "shang", "上尚绱裳上赏晌垧商伤汤殇觞熵墒" },
    { "shao", "勺韶苕杓芍烧稍梢捎鞘蛸筲艄少少绍召稍哨邵捎潲劭" },
    { "she", "奢赊猞畲舍折舌蛇佘社设舍涉射摄赦慑麝滠歙厍" },
    { "shei", "谁" },
    { "shen", "审沈婶谂哂渖矧身深参申伸绅呻莘娠诜砷糁甚慎渗肾蜃葚胂椹什神甚" },
    { "sheng", "绳渑生声胜升牲甥笙胜圣盛乘剩嵊晟省眚" },
    { "shi", "是事世市士式视似示室势试释适氏饰逝誓嗜侍峙仕恃柿轼拭噬弑谥莳贳铈螫舐筮殖匙师诗失施尸湿狮嘘虱蓍酾鲺使始史驶屎矢豕时十实什识食石拾蚀埘莳炻鲥" },
    { "shou", "手首守艏收受授售瘦寿兽狩绶熟" },
    { "shu", "数属署鼠薯暑蜀黍曙数术树述束竖恕墅漱俞戍庶澍沭丨腧书输殊舒叔疏抒淑梳枢蔬倏菽摅姝纾毹殳疋熟孰赎塾秫" },
    { "shua", "刷刷唰耍" },
    { "shuai", "甩率帅蟀衰摔" },
    { "shuan", "栓拴闩涮" },
    { "shuang", "爽双霜孀泷" },
    { "shui", "说税睡谁水" },
    { "shun", "顺舜瞬吮" },
    { "shuo", "数朔硕烁铄妁蒴槊搠说" },
    { "si", "四似食寺肆伺饲嗣巳祀驷泗俟汜兕姒耜笥思斯司私丝撕厮嘶鸶咝澌缌锶厶蛳死厕" },
    { "song", "送宋诵颂讼松忪淞崧嵩凇菘耸悚怂竦" },
    { "sou", "搜艘馊嗖溲飕锼螋嗽擞擞叟薮嗾瞍" },
    { "su", "俗苏稣酥诉速素肃宿缩塑溯粟簌夙嗉谡僳愫涑蔌觫" },
    { "suan", "酸狻算蒜" },
    { "sui", "随遂隋绥岁碎遂祟隧邃穗燧谇虽尿荽睢眭濉髓" },
    { "sun", "孙荪狲飧损笋榫隼" },
    { "suo", "所索锁琐唢缩莎梭嗦唆挲娑睃桫嗍蓑羧" },
    { "ta", "塔鳎獭踏拓榻嗒蹋沓挞闼漯他她它踏塌遢溻铊趿" },
    { "tai", "台胎苔呔太态泰汰酞肽钛台抬苔邰薹骀炱跆鲐" },
    { "tan", "谈弹坛谭潭覃痰澹檀昙锬镡郯摊贪滩瘫坍探叹炭碳坦毯忐袒钽" },
    { "tang", "堂唐糖膛塘棠搪溏螳瑭樘镗螗饧醣躺倘淌傥帑汤趟铴镗耥羰趟烫" },
    { "tao", "套涛掏滔叨焘韬饕绦逃陶桃淘萄啕洮鼗讨" },
    { "te", "特忑忒慝铽" },
    { "tei", "忒" },
    { "teng", "腾疼藤誊滕" },
    { "ti", "体踢梯剔锑体替涕剃惕屉嚏悌倜逖绨裼提题啼蹄醍绨缇鹈荑" },
    { "tian", "掭田填甜恬佃阗畋钿天添腆舔忝殄" },
    { "tiao", "条调迢鲦苕髫龆蜩笤挑佻祧跳眺粜挑窕" },
    { "tie", "帖餮贴帖萜铁帖" },
    { "ting", "听厅汀烃梃挺艇町铤梃停庭亭婷廷霆蜓葶莛" },
    { "tong", "统筒桶捅侗同童彤铜桐瞳佟酮侗仝垌茼峒潼砼通恫嗵同通痛恸" },
    { "tou", "偷钭透头投骰" },
    { "tu", "吐兔堍菟土吐钍图途徒屠涂荼菟酴突秃凸" },
    { "tuan", "彖团抟疃湍" },
    { "tui", "推忒腿退褪蜕煺颓" },
    { "tun", "屯饨臀囤豚吞暾褪氽" },
    { "tuo", "托脱拖乇妥椭庹陀舵驼砣驮沱跎坨鸵橐佗铊酡柁鼍魄拓唾柝箨" },
    { "u", "" },
    { "v", "" },
    { "wai", "崴外歪" },
    { "wan", "晚碗挽婉惋宛莞娩畹皖绾琬脘菀完玩顽丸纨芄烷万腕蔓湾弯蜿剜豌" },
    { "wang", "往网枉惘罔辋魍汪尢王忘亡芒望王往忘旺妄" },
    { "wei", "为维围唯违韦惟帷帏圩囗潍桅嵬闱沩涠委威微危巍萎偎薇逶煨崴葳隈委伟唯尾玮伪炜纬萎娓苇猥痿韪洧隗诿艉鲔为位未味卫谓遗慰魏蔚畏胃喂尉渭猬軎" },
    { "wen", "温瘟问纹汶璺文闻纹蚊雯璺阌稳吻紊刎" },
    { "weng", "翁嗡瓮蕹蓊" },
    { "wo", "窝涡蜗喔倭挝莴握卧哦渥沃斡幄肟硪龌我哦" },
    { "wu", "五武午舞伍侮捂妩忤鹉牾迕庑怃仵於恶屋污乌巫呜诬兀钨邬圬物务误恶悟乌雾勿坞戊兀晤鹜痦寤骛芴杌焐阢婺鋈无亡吴吾捂毋梧唔芜浯蜈鼯" },
    { "xi", "系细戏隙饩阋禊舄喜洗禧徙玺屣葸蓰铣西息希吸惜稀悉析夕牺腊昔熙兮溪嘻锡晰樨熄膝栖郗犀曦奚羲唏蹊淅皙汐嬉茜熹烯翕蟋歙浠僖穸蜥螅菥舾矽粞硒醯欷鼷席习袭媳檄隰觋" },
    { "xia", "瞎虾呷下夏吓厦唬罅峡侠狭霞暇辖遐匣黠瑕狎硖瘕柙" },
    { "xian", "闲贤嫌咸弦娴衔涎舷鹇痫先鲜仙掀纤暹莶锨氙祆籼酰跹现见线限县献宪陷羡馅腺岘苋霰显险鲜洗跣猃藓铣燹蚬筅冼" },
    { "xiang", "想响享飨饷鲞降详祥翔庠相向象像项巷橡蟓相香乡箱厢湘镶襄骧葙芗缃" },
    { "xiao", "笑校效肖孝啸消销潇肖萧宵削嚣逍硝霄哮枭骁箫枵哓蛸绡魈淆崤小晓筱" },
    { "xie", "写解谢泄契械屑卸懈泻亵蟹邂榭瀣薤燮躞廨绁渫榍獬写血叶协鞋携斜胁谐邪挟偕撷勰颉缬些歇楔蝎" },
    { "xin", "信芯衅囟心新欣辛薪馨鑫芯昕忻歆锌寻镡" },
    { "xing", "兴星腥惺猩行形型刑邢陉荥饧硎省醒擤性兴姓幸杏悻荇" },
    { "xiong", "雄熊兄胸凶匈汹芎" },
    { "xiu", "秀袖宿臭绣锈嗅岫溴修休羞咻馐庥鸺貅髹宿朽" },
    { "xu", "续序绪蓄叙畜恤絮旭婿酗煦洫溆勖徐蓿许浒栩诩糈醑需须虚吁嘘墟戌胥砉圩盱顼" },
    { "xuan", "旋券炫渲绚眩铉泫碹楦镟选癣旋悬玄漩璇痃宣喧轩萱暄谖揎儇煊" },
    { "xue", "血谑削靴薛雪鳕学穴噱踅泶" },
    { "xun", "训迅讯逊熏殉巽徇汛蕈浚寻询巡循旬驯荀峋洵恂郇浔鲟熏勋荤醺薰埙曛窨獯" },
    { "ya", "压雅呀押鸦哑鸭丫垭桠呀牙涯崖芽衙睚伢岈琊蚜亚压讶轧娅迓揠氩砑雅瞧匹痖疋" },
    { "yan", "研验沿厌燕宴咽雁焰艳谚彦焱晏唁砚堰赝餍滟酽谳言严研延沿颜炎阎盐岩铅蜒檐妍筵芫闫阽烟燕咽殷焉淹阉腌嫣胭湮阏鄢菸崦恹眼演掩衍奄俨偃魇鼹兖郾琰罨厣剡鼽" },
    { "yang", "养仰痒氧洋阳杨扬羊疡佯烊徉炀蛘样漾恙烊怏鞅央泱秧鸯殃鞅" },
    { "yao", "要约邀腰夭妖吆幺咬杳窈舀崾摇遥姚陶尧谣瑶窑肴侥铫珧轺爻徭繇鳐要药耀钥鹞曜疟" },
    { "ye", "业夜叶页液咽哗曳拽烨掖腋谒邺靥晔耶噎椰掖也野冶爷耶邪揶铘" },
    { "yi", "意义议易衣艺译异益亦亿忆谊抑翼役艾溢毅裔逸轶弈翌疫绎佚奕熠诣弋驿懿呓屹薏噫镒缢邑臆刈羿仡峄怿悒肄佾殪挹埸劓镱瘗癔翊蜴嗌翳一医衣依椅伊漪咿揖噫猗壹铱欹黟移疑遗宜仪蛇姨夷怡颐彝咦贻迤痍胰沂饴圯荑诒眙嶷以已衣尾椅矣乙蚁倚迤蛾旖苡钇舣酏" },
    { "yin", "引隐饮瘾殷尹蚓吲因音烟阴姻殷茵荫喑湮氤堙洇铟银吟寅淫垠鄞霪狺夤圻龈印饮荫胤茚窨" },
    { "ying", "影颖颍瘿郢营迎赢盈蝇莹荧萤萦瀛楹嬴茔滢潆荥蓥应英鹰婴樱膺莺罂鹦缨瑛璎撄嘤应硬映媵" },
    { "yo", "育哟唷哟" },
    { "yong", "用佣喁永勇涌踊泳咏俑恿甬蛹拥庸佣雍臃邕镛墉慵痈壅鳙饔" },
    { "you", "有又右幼诱佑柚囿鼬宥侑蚴釉优幽忧悠攸呦有友黝酉莠牖铕卣由游油邮尤犹柚鱿莸尢铀猷疣蚰蝣蝤繇莜" },
    { "yu", "与语育遇狱雨欲预玉愈谷域誉吁蔚寓豫粥郁喻裕浴御驭尉谕毓妪峪芋昱煜熨燠菀蓣饫阈鬻聿钰鹆鹬蜮于与余予鱼愚舆娱愉馀逾渔渝俞萸瑜隅揄榆虞禺谀腴竽妤臾欤觎盂窬蝓嵛狳舁雩与语雨予宇羽禹圄屿龉伛圉庾瘐窳俣於吁迂淤纡瘀" },
    { "yuan", "院愿怨苑媛掾垸瑗员元原园源圆缘援袁猿垣辕沅媛芫橼圜塬爰螈鼋远冤渊鸳眢鸢箢" },
    { "yue", "说月乐越阅跃悦岳粤钥刖瀹栎樾龠钺约曰" },
    { "yun", "允陨殒狁员运均韵晕孕蕴酝愠熨郓韫恽晕氲员云匀筠芸耘纭昀郧" },
    { "za", "咋扎咂匝拶杂咱砸" },
    { "zai", "载仔宰崽灾哉栽甾在再载" },
    { "zan", "咱攒拶昝趱簪糌赞暂瓒錾" },
    { "zang", "赃臧锗驵藏脏葬奘" },
    { "zao", "凿遭糟造灶躁噪皂燥唣早澡枣蚤藻缲" },
    { "ze", "则责泽择咋啧迮帻赜笮箦舴侧仄昃" },
    { "zei", "贼" },
    { "zen", "谮怎" },
    { "zeng", "赠综缯甑锃曾增憎缯罾" },
    { "zha", "眨砟查扎咋渣喳揸楂哳吒齄炸咋诈乍蜡栅榨柞吒咤痄蚱炸扎札喋轧闸铡" },
    { "zhai", "窄摘侧斋债祭寨砦瘵择宅翟" },
    { "zhan", "占沾粘瞻詹毡谵旃展斩辗盏崭搌战站占颤绽湛蘸栈" },
    { "zhang", "丈涨帐障账胀仗杖瘴嶂幛张章彰璋蟑樟漳嫜鄣獐长掌涨仉" },
    { "zhao", "照赵召罩兆肇诏棹笊着找爪沼着招朝嘲昭钊啁" },
    { "zhe", "者褶锗赭着折哲辙辄谪蛰摺磔蜇这浙蔗鹧柘折遮蜇" },
    { "zhei", "这" },
    { "zhen", "诊枕疹缜畛轸稹阵镇震圳振赈朕鸩真针珍斟贞侦甄臻箴砧桢溱蓁椹榛胗祯浈" },
    { "zheng", "政正证挣郑症怔铮诤帧整拯正争征丁挣症睁徵蒸怔筝铮峥狰钲鲭" },
    { "zhi", "只指纸止址旨徵趾咫芷枳祉轵黹酯直指职值执植殖侄踯摭絷跖埴之只知指支织氏枝汁掷芝吱肢脂蜘栀卮胝祗知至制识治志致质智置秩滞帜稚挚掷峙窒炙痔栉桎帙轾贽痣豸陟忮彘膣雉鸷骘蛭踬郅觯" },
    { "zhong", "中种重众仲种肿踵冢中终钟忠衷锺盅忪螽舯" },
    { "zhou", "周州洲粥舟诌啁皱骤轴宙咒昼胄纣绉荮籀繇酎肘帚轴妯碡" },
    { "zhu", "住注助著驻祝筑柱铸伫贮箸炷蛀杼翥苎疰诸朱珠猪株蛛洙诛铢茱邾潴槠橥侏主属煮嘱瞩拄褚渚麈术逐筑竹烛躅竺舳瘃" },
    { "zhua", "抓挝爪" },
    { "zhuai", "拽曳拽嘬转" },
    { "zhuan", "传转赚撰沌篆啭馔专砖颛转" },
    { "zhuang", "装庄妆桩状壮撞幢僮戆奘" },
    { "zhui", "坠缀赘惴缒追锥隹椎骓" },
    { "zhun", "准屯谆肫窀" },
    { "zhuo", "着著琢缴灼酌浊濯茁啄斫镯诼禚擢浞桌捉卓拙涿焯倬" },
    { "zi", "子紫仔梓姊籽滓秭笫耔茈訾资咨滋仔姿吱兹孜谘呲龇锱辎淄髭赀孳粢趑觜訾缁鲻嵫自字渍恣眦" },
    { "zong", "宗踪综棕鬃枞腙总偬纵粽" },
    { "zou", "奏揍走邹诹陬鄹驺鲰" },
    { "zu", "组祖阻诅俎租菹足族卒镞" },
    { "zuan", "赚钻攥钻躜纂缵" },
    { "zui", "最罪醉蕞堆嘴咀觜" },
    { "zun", "撙尊遵樽鳟" },
    { "zuo", "作做坐座凿柞怍胙阼唑祚酢作昨琢笮左佐撮作嘬" },
    {NULL, NULL}
};
#endif


/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
lv_obj_t * lv_ime_pinyin_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}


/*=====================
 * Setter functions
 *====================*/

/**
 * Set the keyboard of Pinyin input method.
 * @param obj  pointer to a Pinyin input method object
 * @param dict pointer to a Pinyin input method keyboard
 */
void lv_ime_pinyin_set_keyboard(lv_obj_t * obj, lv_obj_t * kb)
{
    if(kb) {
        LV_ASSERT_OBJ(kb, &lv_keyboard_class);
    }

    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    pinyin_ime->kb = kb;
    lv_obj_set_parent(obj, lv_obj_get_parent(kb));
    lv_obj_set_parent(pinyin_ime->cand_panel, lv_obj_get_parent(kb));
    lv_obj_add_event(pinyin_ime->kb, lv_ime_pinyin_kb_event, LV_EVENT_VALUE_CHANGED, obj);
    lv_obj_align_to(pinyin_ime->cand_panel, pinyin_ime->kb, LV_ALIGN_OUT_TOP_MID, 0, 0);
}

/**
 * Set the dictionary of Pinyin input method.
 * @param obj  pointer to a Pinyin input method object
 * @param dict pointer to a Pinyin input method dictionary
 */
void lv_ime_pinyin_set_dict(lv_obj_t * obj, lv_pinyin_dict_t * dict)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    init_pinyin_dict(obj, dict);
}

/**
 * Set mode, 26-key input(k26) or 9-key input(k9).
 * @param obj  pointer to a Pinyin input method object
 * @param mode   the mode from 'lv_keyboard_mode_t'
 */
void lv_ime_pinyin_set_mode(lv_obj_t * obj, lv_ime_pinyin_mode_t mode)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    LV_ASSERT_OBJ(pinyin_ime->kb, &lv_keyboard_class);

    pinyin_ime->mode = mode;

#if LV_IME_PINYIN_USE_K9_MODE
    if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) {
        pinyin_k9_init_data(obj);
        lv_keyboard_set_map(pinyin_ime->kb, LV_KEYBOARD_MODE_USER_1, (const char **)lv_btnm_def_pinyin_k9_map,
                            default_kb_ctrl_k9_map);
        lv_keyboard_set_mode(pinyin_ime->kb, LV_KEYBOARD_MODE_USER_1);
    }
#endif
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Set the dictionary of Pinyin input method.
 * @param obj  pointer to a Pinyin IME object
 * @return     pointer to the Pinyin IME keyboard
 */
lv_obj_t * lv_ime_pinyin_get_kb(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    return pinyin_ime->kb;
}

/**
 * Set the dictionary of Pinyin input method.
 * @param obj  pointer to a Pinyin input method object
 * @return     pointer to the Pinyin input method candidate panel
 */
lv_obj_t * lv_ime_pinyin_get_cand_panel(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    return pinyin_ime->cand_panel;
}

/**
 * Set the dictionary of Pinyin input method.
 * @param obj  pointer to a Pinyin input method object
 * @return     pointer to the Pinyin input method dictionary
 */
lv_pinyin_dict_t * lv_ime_pinyin_get_dict(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    return pinyin_ime->dict;
}

/*=====================
 * Other functions
 *====================*/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_ime_pinyin_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    uint16_t py_str_i = 0;
    uint16_t btnm_i = 0;
    for(btnm_i = 0; btnm_i < (LV_IME_PINYIN_CAND_TEXT_NUM + 3); btnm_i++) {
        if(btnm_i == 0) {
            lv_btnm_def_pinyin_sel_map[btnm_i] = "<";
        }
        else if(btnm_i == (LV_IME_PINYIN_CAND_TEXT_NUM + 1)) {
            lv_btnm_def_pinyin_sel_map[btnm_i] = ">";
        }
        else if(btnm_i == (LV_IME_PINYIN_CAND_TEXT_NUM + 2)) {
            lv_btnm_def_pinyin_sel_map[btnm_i] = "";
        }
        else {
            lv_pinyin_cand_str[py_str_i][0] = ' ';
            lv_btnm_def_pinyin_sel_map[btnm_i] = lv_pinyin_cand_str[py_str_i];
            py_str_i++;
        }
    }

    pinyin_ime->mode = LV_IME_PINYIN_MODE_K26;
    pinyin_ime->py_page = 0;
    pinyin_ime->ta_count = 0;
    pinyin_ime->cand_num = 0;
    lv_memzero(pinyin_ime->input_char, sizeof(pinyin_ime->input_char));
    lv_memzero(pinyin_ime->py_num, sizeof(pinyin_ime->py_num));
    lv_memzero(pinyin_ime->py_pos, sizeof(pinyin_ime->py_pos));

    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);

#if LV_IME_PINYIN_USE_DEFAULT_DICT
    init_pinyin_dict(obj, lv_ime_pinyin_def_dict);
#endif

    /* Init pinyin_ime->cand_panel */
    pinyin_ime->cand_panel = lv_btnmatrix_create(lv_obj_get_parent(obj));
    lv_btnmatrix_set_map(pinyin_ime->cand_panel, (const char **)lv_btnm_def_pinyin_sel_map);
    lv_obj_set_size(pinyin_ime->cand_panel, LV_PCT(100), LV_PCT(5));
    lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);

    lv_btnmatrix_set_one_checked(pinyin_ime->cand_panel, true);
    lv_obj_clear_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_CLICK_FOCUSABLE);

    /* Set cand_panel style*/
    // Default style
    lv_obj_set_style_bg_opa(pinyin_ime->cand_panel, LV_OPA_0, 0);
    lv_obj_set_style_border_width(pinyin_ime->cand_panel, 0, 0);
    lv_obj_set_style_pad_all(pinyin_ime->cand_panel, 8, 0);
    lv_obj_set_style_pad_gap(pinyin_ime->cand_panel, 0, 0);
    lv_obj_set_style_radius(pinyin_ime->cand_panel, 0, 0);
    lv_obj_set_style_pad_gap(pinyin_ime->cand_panel, 0, 0);
    lv_obj_set_style_base_dir(pinyin_ime->cand_panel, LV_BASE_DIR_LTR, 0);

    // LV_PART_ITEMS style
    lv_obj_set_style_radius(pinyin_ime->cand_panel, 12, LV_PART_ITEMS);
    lv_obj_set_style_bg_color(pinyin_ime->cand_panel, lv_color_white(), LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(pinyin_ime->cand_panel, LV_OPA_0, LV_PART_ITEMS);
    lv_obj_set_style_shadow_opa(pinyin_ime->cand_panel, LV_OPA_0, LV_PART_ITEMS);

    // LV_PART_ITEMS | LV_STATE_PRESSED style
    lv_obj_set_style_bg_opa(pinyin_ime->cand_panel, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(pinyin_ime->cand_panel, lv_color_white(), LV_PART_ITEMS | LV_STATE_PRESSED);

    /* event handler */
    lv_obj_add_event(pinyin_ime->cand_panel, lv_ime_pinyin_cand_panel_event, LV_EVENT_VALUE_CHANGED, obj);
    lv_obj_add_event(obj, lv_ime_pinyin_style_change_event, LV_EVENT_STYLE_CHANGED, NULL);

#if LV_IME_PINYIN_USE_K9_MODE
    pinyin_ime->k9_input_str_len = 0;
    pinyin_ime->k9_py_ll_pos = 0;
    pinyin_ime->k9_legal_py_count = 0;
    lv_memzero(pinyin_ime->k9_input_str, LV_IME_PINYIN_K9_MAX_INPUT);

    pinyin_k9_init_data(obj);

    _lv_ll_init(&(pinyin_ime->k9_legal_py_ll), sizeof(ime_pinyin_k9_py_str_t));
#endif
}


static void lv_ime_pinyin_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    if(lv_obj_is_valid(pinyin_ime->kb))
        lv_obj_del(pinyin_ime->kb);

    if(lv_obj_is_valid(pinyin_ime->cand_panel))
        lv_obj_del(pinyin_ime->cand_panel);
}


static void lv_ime_pinyin_kb_event(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * kb = lv_event_get_target(e);
    lv_obj_t * obj = lv_event_get_user_data(e);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

#if LV_IME_PINYIN_USE_K9_MODE
    static const char * k9_py_map[8] = {"abc", "def", "ghi", "jkl", "mno", "pqrs", "tuv", "wxyz"};
#endif

    if(code == LV_EVENT_VALUE_CHANGED) {
        uint16_t btn_id  = lv_btnmatrix_get_selected_btn(kb);
        if(btn_id == LV_BTNMATRIX_BTN_NONE) return;

        const char * txt = lv_btnmatrix_get_btn_text(kb, lv_btnmatrix_get_selected_btn(kb));
        if(txt == NULL) return;

        lv_obj_t * ta = lv_keyboard_get_textarea(pinyin_ime->kb);

#if LV_IME_PINYIN_USE_K9_MODE
        if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) {

            uint16_t tmp_btn_str_len = lv_strlen(pinyin_ime->input_char);
            if((btn_id >= 16) && (tmp_btn_str_len > 0) && (btn_id < (16 + LV_IME_PINYIN_K9_CAND_TEXT_NUM))) {
                lv_memzero(pinyin_ime->input_char, sizeof(pinyin_ime->input_char));
                strcat(pinyin_ime->input_char, txt);
                pinyin_input_proc(obj);

                for(int index = 0; index <  tmp_btn_str_len; index++) {
                    lv_textarea_del_char(ta);
                }

                pinyin_ime->ta_count = tmp_btn_str_len;
                pinyin_ime->k9_input_str_len = tmp_btn_str_len;
                lv_textarea_add_text(ta, pinyin_ime->input_char);

                return;
            }
        }
#endif

        if(strcmp(txt, "Enter") == 0 || strcmp(txt, LV_SYMBOL_NEW_LINE) == 0) {
            pinyin_ime_clear_data(obj);
            lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
        }
        else if(strcmp(txt, LV_SYMBOL_BACKSPACE) == 0) {
            // del input char
            if(pinyin_ime->ta_count > 0) {
                if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K26)
                    pinyin_ime->input_char[pinyin_ime->ta_count - 1] = '\0';
#if LV_IME_PINYIN_USE_K9_MODE
                else
                    pinyin_ime->k9_input_str[pinyin_ime->ta_count - 1] = '\0';
#endif

                pinyin_ime->ta_count--;
                if(pinyin_ime->ta_count <= 0) {
                    pinyin_ime_clear_data(obj);
                    lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
                }
                else if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K26) {
                    pinyin_input_proc(obj);
                }
#if LV_IME_PINYIN_USE_K9_MODE
                else if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) {
                    pinyin_ime->k9_input_str_len = lv_strlen(pinyin_ime->input_char) - 1;
                    pinyin_k9_get_legal_py(obj, pinyin_ime->k9_input_str, k9_py_map);
                    pinyin_k9_fill_cand(obj);
                    pinyin_input_proc(obj);
                    pinyin_ime->ta_count--;
                }
#endif
            }
        }
        else if((strcmp(txt, "ABC") == 0) || (strcmp(txt, "abc") == 0) || (strcmp(txt, "1#") == 0) ||
                (strcmp(txt, LV_SYMBOL_OK) == 0)) {
            pinyin_ime_clear_data(obj);
            return;
        }
        else if(strcmp(txt, "123") == 0) {
            for(uint16_t i = 0; i < lv_strlen(txt); i++)
                lv_textarea_del_char(ta);

            pinyin_ime_clear_data(obj);
            lv_textarea_set_cursor_pos(ta, LV_TEXTAREA_CURSOR_LAST);
            lv_ime_pinyin_set_mode(obj, LV_IME_PINYIN_MODE_K9_NUMBER);
            lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);
            lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
        }
        else if(strcmp(txt, LV_SYMBOL_KEYBOARD) == 0) {
            if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K26) {
                lv_ime_pinyin_set_mode(obj, LV_IME_PINYIN_MODE_K9);
            }
            else if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) {
                lv_ime_pinyin_set_mode(obj, LV_IME_PINYIN_MODE_K26);
                lv_keyboard_set_mode(pinyin_ime->kb, LV_KEYBOARD_MODE_TEXT_LOWER);
            }
            else if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K9_NUMBER) {
                lv_ime_pinyin_set_mode(obj, LV_IME_PINYIN_MODE_K9);
            }
            pinyin_ime_clear_data(obj);
        }
        else if((pinyin_ime->mode == LV_IME_PINYIN_MODE_K26) && ((txt[0] >= 'a' && txt[0] <= 'z') || (txt[0] >= 'A' &&
                                                                                                      txt[0] <= 'Z'))) {
            strcat(pinyin_ime->input_char, txt);
            pinyin_input_proc(obj);
            pinyin_ime->ta_count++;
        }
#if LV_IME_PINYIN_USE_K9_MODE
        else if((pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) && (txt[0] >= 'a' && txt[0] <= 'z')) {
            for(uint16_t i = 0; i < 8; i++) {
                if((strcmp(txt, k9_py_map[i]) == 0) || (strcmp(txt, "abc ") == 0)) {
                    if(strcmp(txt, "abc ") == 0)    pinyin_ime->k9_input_str_len += lv_strlen(k9_py_map[i]) + 1;
                    else                            pinyin_ime->k9_input_str_len += lv_strlen(k9_py_map[i]);
                    pinyin_ime->k9_input_str[pinyin_ime->ta_count] = 50 + i;
                    pinyin_ime->k9_input_str[pinyin_ime->ta_count + 1] = '\0';

                    break;
                }
            }
            pinyin_k9_get_legal_py(obj, pinyin_ime->k9_input_str, k9_py_map);
            pinyin_k9_fill_cand(obj);
            pinyin_input_proc(obj);
        }
        else if(strcmp(txt, LV_SYMBOL_LEFT) == 0) {
            pinyin_k9_cand_page_proc(obj, 0);
        }
        else if(strcmp(txt, LV_SYMBOL_RIGHT) == 0) {
            pinyin_k9_cand_page_proc(obj, 1);
        }
#endif
    }
}


static void lv_ime_pinyin_cand_panel_event(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * cand_panel = lv_event_get_target(e);
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_user_data(e);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t * ta = lv_keyboard_get_textarea(pinyin_ime->kb);
        if(ta == NULL) return;

        uint32_t id = lv_btnmatrix_get_selected_btn(cand_panel);
        if(id == LV_BTNMATRIX_BTN_NONE) {
            return;
        }
        else if(id == 0) {
            pinyin_page_proc(obj, 0);
            return;
        }
        else if(id == (LV_IME_PINYIN_CAND_TEXT_NUM + 1)) {
            pinyin_page_proc(obj, 1);
            return;
        }

        const char * txt = lv_btnmatrix_get_btn_text(cand_panel, id);
        uint16_t index = 0;
        for(index = 0; index < pinyin_ime->ta_count; index++)
            lv_textarea_del_char(ta);

        lv_textarea_add_text(ta, txt);

        pinyin_ime_clear_data(obj);
    }
}


static void pinyin_input_proc(lv_obj_t * obj)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    pinyin_ime->cand_str = pinyin_search_matching(obj, pinyin_ime->input_char, &pinyin_ime->cand_num);
    if(pinyin_ime->cand_str == NULL) {
        return;
    }

    pinyin_ime->py_page = 0;

    for(uint8_t i = 0; i < LV_IME_PINYIN_CAND_TEXT_NUM; i++) {
        memset(lv_pinyin_cand_str[i], 0x00, sizeof(lv_pinyin_cand_str[i]));
        lv_pinyin_cand_str[i][0] = ' ';
    }

    // fill buf
    for(uint8_t i = 0; (i < pinyin_ime->cand_num && i < LV_IME_PINYIN_CAND_TEXT_NUM); i++) {
        for(uint8_t j = 0; j < 3; j++) {
            lv_pinyin_cand_str[i][j] = pinyin_ime->cand_str[i * 3 + j];
        }
    }

    lv_obj_clear_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
}

static void pinyin_page_proc(lv_obj_t * obj, uint16_t dir)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;
    uint16_t page_num = pinyin_ime->cand_num / LV_IME_PINYIN_CAND_TEXT_NUM;
    uint16_t sur = pinyin_ime->cand_num % LV_IME_PINYIN_CAND_TEXT_NUM;

    /*if pinyin_ime->cand_str not existed in dict, then do not fill the buffer.
     * - If I input `hu`, it is a legal pinyin (registered in the dictionary);
     * - if I input `hup`, it is not a legal pinyin (not registered in the dictionary),
     *   then not fill the buffer, but returned directly.
     */
    if(!pinyin_ime->cand_str) return;

    if(dir == 0) {
        if(pinyin_ime->py_page) {
            pinyin_ime->py_page--;
        }
    }
    else {
        if(sur == 0) {
            page_num -= 1;
        }
        if(pinyin_ime->py_page < page_num) {
            pinyin_ime->py_page++;
        }
        else return;
    }

    for(uint8_t i = 0; i < LV_IME_PINYIN_CAND_TEXT_NUM; i++) {
        memset(lv_pinyin_cand_str[i], 0x00, sizeof(lv_pinyin_cand_str[i]));
        lv_pinyin_cand_str[i][0] = ' ';
    }

    // fill buf
    uint16_t offset = pinyin_ime->py_page * (3 * LV_IME_PINYIN_CAND_TEXT_NUM);
    for(uint8_t i = 0; (i < pinyin_ime->cand_num && i < LV_IME_PINYIN_CAND_TEXT_NUM); i++) {
        if((sur > 0) && (pinyin_ime->py_page == page_num)) {
            if(i > sur)
                break;
        }
        for(uint8_t j = 0; j < 3; j++) {
            lv_pinyin_cand_str[i][j] = pinyin_ime->cand_str[offset + (i * 3) + j];
        }
    }
}


static void lv_ime_pinyin_style_change_event(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    if(code == LV_EVENT_STYLE_CHANGED) {
        const lv_font_t * font = lv_obj_get_style_text_font(obj, LV_PART_MAIN);
        lv_obj_set_style_text_font(pinyin_ime->cand_panel, font, 0);
    }
}


static void init_pinyin_dict(lv_obj_t * obj, lv_pinyin_dict_t * dict)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    char headletter = 'a';
    uint16_t offset_sum = 0;
    uint16_t offset_count = 0;
    uint16_t letter_calc = 0;

    pinyin_ime->dict = dict;

    for(uint16_t i = 0; ; i++) {
        if((NULL == (dict[i].py)) || (NULL == (dict[i].py_mb))) {
            headletter = dict[i - 1].py[0];
            letter_calc = headletter - 'a';
            pinyin_ime->py_num[letter_calc] = offset_count;
            break;
        }

        if(headletter == (dict[i].py[0])) {
            offset_count++;
        }
        else {
            headletter = dict[i].py[0];
            letter_calc = headletter - 'a';
            pinyin_ime->py_num[letter_calc - 1] = offset_count;
            offset_sum += offset_count;
            pinyin_ime->py_pos[letter_calc] = offset_sum;

            offset_count = 1;
        }
    }
}


static char * pinyin_search_matching(lv_obj_t * obj, char * py_str, uint16_t * cand_num)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    lv_pinyin_dict_t * cpHZ;
    uint8_t index, len = 0, offset;
    volatile uint8_t count = 0;

    if(*py_str == '\0')    return NULL;
    if(*py_str == 'i')     return NULL;
    if(*py_str == 'u')     return NULL;
    if(*py_str == 'v')     return NULL;

    offset = py_str[0] - 'a';
    len = lv_strlen(py_str);

    cpHZ  = &pinyin_ime->dict[pinyin_ime->py_pos[offset]];
    count = pinyin_ime->py_num[offset];

    while(count--) {
        for(index = 0; index < len; index++) {
            if(*(py_str + index) != *((cpHZ->py) + index)) {
                break;
            }
        }

        // perfect match
        if(len == 1 || index == len) {
            // The Chinese character in UTF-8 encoding format is 3 bytes
            * cand_num = lv_strlen((const char *)(cpHZ->py_mb)) / 3;
            return (char *)(cpHZ->py_mb);
        }
        cpHZ++;
    }
    return NULL;
}

static void pinyin_ime_clear_data(lv_obj_t * obj)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

#if LV_IME_PINYIN_USE_K9_MODE
    if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) {
        pinyin_ime->k9_input_str_len = 0;
        pinyin_ime->k9_py_ll_pos = 0;
        pinyin_ime->k9_legal_py_count = 0;
        lv_memzero(pinyin_ime->k9_input_str,  LV_IME_PINYIN_K9_MAX_INPUT);
        lv_memzero(lv_pinyin_k9_cand_str, sizeof(lv_pinyin_k9_cand_str));
        lv_strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM], LV_SYMBOL_RIGHT"\0");
        lv_strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1], "\0");
    }
#endif

    pinyin_ime->ta_count = 0;
    lv_memzero(lv_pinyin_cand_str, (sizeof(lv_pinyin_cand_str)));
    lv_memzero(pinyin_ime->input_char, sizeof(pinyin_ime->input_char));

    lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
}


#if LV_IME_PINYIN_USE_K9_MODE
static void pinyin_k9_init_data(lv_obj_t * obj)
{
    LV_UNUSED(obj);

    uint16_t py_str_i = 0;
    uint16_t btnm_i = 0;
    for(btnm_i = 19; btnm_i < (LV_IME_PINYIN_K9_CAND_TEXT_NUM + 21); btnm_i++) {
        if(py_str_i == LV_IME_PINYIN_K9_CAND_TEXT_NUM) {
            lv_strcpy(lv_pinyin_k9_cand_str[py_str_i], LV_SYMBOL_RIGHT"\0");
        }
        else if(py_str_i == LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1) {
            lv_strcpy(lv_pinyin_k9_cand_str[py_str_i], "\0");
        }
        else {
            lv_strcpy(lv_pinyin_k9_cand_str[py_str_i], " \0");
        }

        lv_btnm_def_pinyin_k9_map[btnm_i] = lv_pinyin_k9_cand_str[py_str_i];
        py_str_i++;
    }

    default_kb_ctrl_k9_map[0]  = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
    default_kb_ctrl_k9_map[4]  = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
    default_kb_ctrl_k9_map[5]  = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
    default_kb_ctrl_k9_map[9]  = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
    default_kb_ctrl_k9_map[10] = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
    default_kb_ctrl_k9_map[14] = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
    default_kb_ctrl_k9_map[15] = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
    default_kb_ctrl_k9_map[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 16] = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
}

static void pinyin_k9_get_legal_py(lv_obj_t * obj, char * k9_input, const char * py9_map[])
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    uint16_t len = lv_strlen(k9_input);

    if((len == 0) || (len >= LV_IME_PINYIN_K9_MAX_INPUT)) {
        return;
    }

    char py_comp[LV_IME_PINYIN_K9_MAX_INPUT] = {0};
    int mark[LV_IME_PINYIN_K9_MAX_INPUT] = {0};
    int index = 0;
    int flag = 0;
    uint16_t count = 0;

    uint32_t ll_len = 0;
    ime_pinyin_k9_py_str_t * ll_index = NULL;

    ll_len = _lv_ll_get_len(&pinyin_ime->k9_legal_py_ll);
    ll_index = _lv_ll_get_head(&pinyin_ime->k9_legal_py_ll);

    while(index != -1) {
        if(index == len) {
            if(pinyin_k9_is_valid_py(obj, py_comp)) {
                if((count >= ll_len) || (ll_len == 0)) {
                    ll_index = _lv_ll_ins_tail(&pinyin_ime->k9_legal_py_ll);
                    lv_strcpy(ll_index->py_str, py_comp);
                }
                else if((count < ll_len)) {
                    lv_strcpy(ll_index->py_str, py_comp);
                    ll_index = _lv_ll_get_next(&pinyin_ime->k9_legal_py_ll, ll_index);
                }
                count++;
            }
            index--;
        }
        else {
            flag = mark[index];
            if((size_t)flag < lv_strlen(py9_map[k9_input[index] - '2'])) {
                py_comp[index] = py9_map[k9_input[index] - '2'][flag];
                mark[index] = mark[index] + 1;
                index++;
            }
            else {
                mark[index] = 0;
                index--;
            }
        }
    }

    if(count > 0) {
        pinyin_ime->ta_count++;
        pinyin_ime->k9_legal_py_count = count;
    }
}


/*true: visible; false: not visible*/
static bool pinyin_k9_is_valid_py(lv_obj_t * obj, char * py_str)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    lv_pinyin_dict_t * cpHZ = NULL;
    uint8_t index = 0, len = 0, offset = 0;
    volatile uint8_t count = 0;

    if(*py_str == '\0')    return false;
    if(*py_str == 'i')     return false;
    if(*py_str == 'u')     return false;
    if(*py_str == 'v')     return false;

    offset = py_str[0] - 'a';
    len = lv_strlen(py_str);

    cpHZ  = &pinyin_ime->dict[pinyin_ime->py_pos[offset]];
    count = pinyin_ime->py_num[offset];

    while(count--) {
        for(index = 0; index < len; index++) {
            if(*(py_str + index) != *((cpHZ->py) + index)) {
                break;
            }
        }

        // perfect match
        if(len == 1 || index == len) {
            return true;
        }
        cpHZ++;
    }
    return false;
}


static void pinyin_k9_fill_cand(lv_obj_t * obj)
{
    static uint16_t len = 0;
    uint16_t index = 0, tmp_len = 0;
    ime_pinyin_k9_py_str_t * ll_index = NULL;

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    tmp_len = pinyin_ime->k9_legal_py_count;

    if(tmp_len != len) {
        lv_memzero(lv_pinyin_k9_cand_str, sizeof(lv_pinyin_k9_cand_str));
        lv_strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM], LV_SYMBOL_RIGHT"\0");
        lv_strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1], "\0");
        len = tmp_len;
    }

    ll_index = _lv_ll_get_head(&pinyin_ime->k9_legal_py_ll);
    lv_strcpy(pinyin_ime->input_char, ll_index->py_str);
    while(ll_index) {
        if((index >= LV_IME_PINYIN_K9_CAND_TEXT_NUM) || \
           (index >= pinyin_ime->k9_legal_py_count))
            break;

        lv_strcpy(lv_pinyin_k9_cand_str[index], ll_index->py_str);
        ll_index = _lv_ll_get_next(&pinyin_ime->k9_legal_py_ll, ll_index); /*Find the next list*/
        index++;
    }
    pinyin_ime->k9_py_ll_pos = index;

    lv_obj_t * ta = lv_keyboard_get_textarea(pinyin_ime->kb);
    for(index = 0; index < pinyin_ime->k9_input_str_len; index++) {
        lv_textarea_del_char(ta);
    }
    pinyin_ime->k9_input_str_len = lv_strlen(pinyin_ime->input_char);
    lv_textarea_add_text(ta, pinyin_ime->input_char);
}


static void pinyin_k9_cand_page_proc(lv_obj_t * obj, uint16_t dir)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    lv_obj_t * ta = lv_keyboard_get_textarea(pinyin_ime->kb);
    uint16_t ll_len =  _lv_ll_get_len(&pinyin_ime->k9_legal_py_ll);

    if((ll_len > LV_IME_PINYIN_K9_CAND_TEXT_NUM) && (pinyin_ime->k9_legal_py_count > LV_IME_PINYIN_K9_CAND_TEXT_NUM)) {
        ime_pinyin_k9_py_str_t * ll_index = NULL;
        int count = 0;

        ll_index = _lv_ll_get_head(&pinyin_ime->k9_legal_py_ll);
        while(ll_index) {
            if(count >= pinyin_ime->k9_py_ll_pos)   break;

            ll_index = _lv_ll_get_next(&pinyin_ime->k9_legal_py_ll, ll_index); /*Find the next list*/
            count++;
        }

        if((NULL == ll_index) && (dir == 1))   return;

        lv_memzero(lv_pinyin_k9_cand_str, sizeof(lv_pinyin_k9_cand_str));
        lv_strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM], LV_SYMBOL_RIGHT"\0");
        lv_strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1], "\0");

        // next page
        if(dir == 1) {
            count = 0;
            while(ll_index) {
                if(count >= (LV_IME_PINYIN_K9_CAND_TEXT_NUM - 1))
                    break;

                lv_strcpy(lv_pinyin_k9_cand_str[count], ll_index->py_str);
                ll_index = _lv_ll_get_next(&pinyin_ime->k9_legal_py_ll, ll_index); /*Find the next list*/
                count++;
            }
            pinyin_ime->k9_py_ll_pos += count - 1;

        }
        // previous page
        else {
            count = LV_IME_PINYIN_K9_CAND_TEXT_NUM - 1;
            ll_index = _lv_ll_get_prev(&pinyin_ime->k9_legal_py_ll, ll_index);
            while(ll_index) {
                if(count < 0)  break;

                lv_strcpy(lv_pinyin_k9_cand_str[count], ll_index->py_str);
                ll_index = _lv_ll_get_prev(&pinyin_ime->k9_legal_py_ll, ll_index); /*Find the previous list*/
                count--;
            }

            if(pinyin_ime->k9_py_ll_pos > LV_IME_PINYIN_K9_CAND_TEXT_NUM)
                pinyin_ime->k9_py_ll_pos -= 1;
        }

        lv_textarea_set_cursor_pos(ta, LV_TEXTAREA_CURSOR_LAST);
    }
}

#endif  /*LV_IME_PINYIN_USE_K9_MODE*/

#endif  /*LV_USE_IME_PINYIN*/
