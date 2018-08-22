/*
 * Platform.h
 *
 * SDK接口文件，定义了全部结构体及绝大部分接口
 * 用户使用SDK需要包含该文件
 *
 * By Berlin 2018.08 <chenbl@yumair.cn>
 */
#ifndef PLATFORM_H
#define PLATFORM_H

#define PROPERTY_INVALID_APPID 0xffff //无效的APPID

typedef unsigned char ptBool_t;

/*
* 公共属性信息
*/
typedef struct
{
    ptBool_t isText; //是否为文本
    char *name;
    union
    {
        int num;
        char *text;
    }value;
}PCommonInfo_t;

/*
* 升级固件数据
*/
typedef struct
{
    unsigned char *data;   //当前数据内容
    unsigned int datalen;  //当前数据大小
    unsigned int fileSize; //总的文件大小
}POTADataArgs_t;

/*
* 故障上报
*/
typedef struct
{
    unsigned short serialId; //多路设备的路数，0表示单路设备
    int errCode;             //故障码
    char *errInfo;           //描述信息
    unsigned int time;       //故障发生utc时间
}PErrorReport_t;

/*
* 状态告警信息
*/
typedef struct
{
    unsigned short serialId;  //多路设备的路数，0表示单路设备
    PCommonInfo_t statusInfo; //告警的设备属性信息
    char *description;        //描述
    unsigned int time;        //告警发生utc时间
}PStatusAlarm_t;

/*
* 语音控制扩展字段(可选)
* key/value 数据组
*/
typedef struct
{
    char *key;
    char *value;
}PVoiceCtrlExtension_t;

/*
* 语音控制结果
*/
typedef struct
{
    char *did;  //主/从设备ID
    char *rawText; //语音识别转换成的文本
    char *description; //结果描述
}PVoiceCtrlResult_t;

/*
* 语音合成结果
*/
typedef struct
{
    char *did; //主/从设备ID
    char *content; //合成的音频数据BASE64编码
}PTTSResult_t;

/*
* 语音合成参数
*/
typedef struct
{
    char *content;        //语音合成文本内容
    char *anchor;         //发音人标识(若空则为默认中文发音人)
    unsigned char speed;  //语速(0~100)
    unsigned char volume; //音量(0~100)
    unsigned char pitch;  //音调(0~100)
    char *format;         //语音编码格式(pcm\speex\amr)
    unsigned short rate;  //采样率(8000\16000)
    unsigned char channel; //声道(1:单声道)
}PTTSParameter_t;

/*
语音控制参数
*/
typedef struct
{
    char *format;             //语音数据压缩格式:"pcm","speex","amr"
    unsigned short rate;      //语音数据采样率，取值：8000,16000
    unsigned char channel;    //语音数据声道: 1：单声道
    char *voiceID;            //语音数据编号,在录音数据分段录制并分段传输时进行赋值，一次录音过程该编号相同，格式：deviceId+时间戳
    unsigned short sn;        //每个分段录音数据的顺序号，从 1 开始递增，需要设备按照分段顺序上传，语音分段上传时，sequence 使用同一个
}PVoiceCtrlParameter_t;

/*
* 信息/事件
*/
typedef struct
{
    char *name; //名称
    unsigned short serialID; //多路设备的路数，0表示单路设备
    unsigned char infoNum; //信息个数
    PCommonInfo_t *info;   //信息内容
}PEventInfo_t;

/*
* 升级信息
*/
typedef struct
{
    unsigned char type; //0：给设备升级, 1：给模块升级
    char *name; //固件名称，升级设备为空
    char *url;  //连接地址
    char *version; //版本号
    char *md5;  //MD5校验值
}PUpgradeInfo_t;

/*
* 升级通知
*/
typedef struct
{
    char *did;   //设备ID
    int infoNum; //信息个数(有可能同时有多个固件信息，设备从中选择一个)
    PUpgradeInfo_t *info; //信息内容
}PUpgradeNotice_t;

/*
* 资源
*/
typedef struct
{
    unsigned short sid; //多路设备的路数，0表示单路设备
    char *did;     //设备ID
    char *rscName; //资源名
    unsigned short rscID; //资源ID
    int infoNum;
    PCommonInfo_t *info;
}PResources_t;

typedef struct
{
    ptBool_t isText;
    unsigned short appid;
    union
    {
        int num;
        char *text;
    }value;
}PPropertyValue_t;

/*
* 设备状态控制
*/
typedef struct
{
    char *did; //设备ID
    unsigned short sid; //多路设备的路数，0表示单路设备
    int propertyNum; //属性个数
    PPropertyValue_t *property; //属性
}PPropertyCtrl_t;

typedef struct PPropertyInfo_st
{
    char *name;           //属性名称
    ptBool_t isText;      //是否为文本属性
    ptBool_t readonly;    //是否为只读
    unsigned short appid; //app对应的属性ID（用户自己定义，每个属性appid唯一）
    unsigned short sid;   //多路设备的路数，0 表示为单路设备，1、2、3 表示多路设备的第 1、2、3 路
}PPropertyInfo_t;

/*平台主结构体*/
struct PrivateCtx_st;
typedef struct PlatformCtx_st
{
    struct PrivateCtx_st *private;
}PlatformCtx_t;

typedef enum
{
    PEVENT_SERVER_ON_OFFLINE,     //服务器上、下线状态
    PEVENT_PROPERTY_CONTRL,       //设备、子设备状态控制
    PEVENT_RESOURCE_ADD,          //设备、子设备资源新增
    PEVENT_RESOURCE_SET,          //设备、子设备资源设置
    PEVENT_RESOURCE_DEL,          //设备、子设备资源删除
    PEVENT_SUBDEV_AUTH_FAILED,    //子设备鉴权失败
    PEVENT_SUBDEV_UNBINDED,       //子设备被解绑
    PEVENT_TIMING_INFO,           //授时信息
    PEVENT_OTA_NOTICE,            //升级通知
    PEVENT_OTA_DATA,              //升级数据
    PEVENT_OTA_FINISH,            //升级完成
    PEVENT_VOICE_CONTROL_RESULT,  //语音控制结果
    PEVENT_TTS_RESULT,            //语音合成结果
}PlatformEvent_t;

/*事件回调函数*/
typedef void(*PlatformEventHandle_t)(PlatformEvent_t event, void *args);

/*
* 语音合成
* 通过该接口向智能家居平台发起语音合成请求，输入文本获取到相应语音数据
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*      @did:主/从设备ID
*      @param:语音合成参数
*/
void PlatformTTS(PlatformCtx_t *ctx, const char *did, PTTSParameter_t *param);

/*
* 语音控制
* 设备上传的语音数据，平台进行语音识别并得到设备控制的指令（支持分段接收语音数据）
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*      @did:主/从设备ID
*      @param:语音参数
*      @data:语音数据
*      @isLast:是否为最后一段数据，1：是最后一段数据，0：不是最后一段数据
*      @ext:扩展字段（可选，不使用为NULL）
*      @extNum:扩展字段结构体个数（不使用扩展字段为0）
*/
void PlatformVoiceControl(PlatformCtx_t *ctx, const char *did, PVoiceCtrlParameter_t *param, const char *data, ptBool_t isLast, PVoiceCtrlExtension_t *ext, int extNum);

/*
* 设备故障反馈
* 当设备或其子设备发生故障时，设备会通过本接口消息发送故障代码列表给智能家居平台
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*      @did:主/从设备ID
*      @error:故障信息
*/
void PlatformErrorReport(PlatformCtx_t *ctx, const char *did, PErrorReport_t *error);

/*
* 状态告警
* 设备检测到设备属性超过设定的阀值时，通过该接口发送告警数据到智能家居平台
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*      @did:主/从设备ID
*      @alarm:告警信息
*/
void PlatformStatusAlarm(PlatformCtx_t *ctx, const char *did, PStatusAlarm_t *alarm);

/*
* 设备信息上报
* 设备运行过程中产生的信息/事件（如闹铃、开门）可通过该接口上报
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*      @did:主/从设备ID
*      @alarm:信息内容
*/
void PlatformEventInfoReport(PlatformCtx_t *ctx, const char *did, PEventInfo_t *event);

/*------------------------------设备资源--------------------------------*/

/*
* 设备资源修改
* 设备本地修改了资源内容，通过该接口上报给平台
* 参数: @ctx:主结构体(PlatformCtxCreate返回)
*      @did:主/从设备ID
*      @rscName:资源种类名称
*      @id:资源ID
*      @value:修改内容
*      @valNum:内容的个数
* 返回值:0:成功，< 0:失败
*/
int PlatformResourceItemSet(PlatformCtx_t *ctx, const char *did, const char *rscName, unsigned short id, PCommonInfo_t *value, int valNum);

/*
* 设备资源删除
* 设备本地删除了资源内容，通过该接口上报给平台
* 参数: @ctx:主结构体(PlatformCtxCreate返回)
*      @did:主/从设备ID
*      @rscName:资源种类名称
*      @id:资源ID
* 返回值:0:成功，< 0:失败
*/
int PlatformResourceItemDel(PlatformCtx_t *ctx, const char *did, const char *rscName, unsigned short id);

/*
* 设备资源增加
* 设备本地增加了新的资源，通过该接口上报给平台
* 参数: @ctx:主结构体(PlatformCtxCreate返回)
*      @did:主/从设备ID
*      @rscName:资源种类名称
*      @id:资源ID
*      @value:资源内容
*      @valNum:内容的个数
* 返回值:0:成功，< 0:失败
*/
int PlatformResourceItemAdd(PlatformCtx_t *ctx, const char *did, const char *rscName, unsigned short id, PCommonInfo_t *value, int valNum);

/*
* 注册资源名称关键字
* 需要事先通过该接口注册除ID名称之外的资源名称
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*      @did:主/从设备ID
*      @rscName:资源种类名称
*      @keyword:资源关键字名称
*      @valueIsText:该资源的值是否为字符，true：字符，false：数值
* 返回值:0:成功，< 0:失败
*/
int PlatformResourceKeywordRegister(PlatformCtx_t *ctx, const char *did, const char *rscName, const char *keyword, ptBool_t valueIsText);

/*
* 资源注册
* 任何操作资源之前需要先组册资源内容
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*      @did:主/从设备ID
*      @rscName:资源种类名称
*      @idName:资源ID名称
*      @serialId:多路设备的路数，0表示单路设备
* 返回值:0:成功，< 0:失败
*/
int PlatformResourceRegister(PlatformCtx_t *ctx, const char *did, const char *rscName, const char *idName, unsigned short serialId);

/*------------------------------设备状态(属性)--------------------------------------*/

/*
* 设置设备状态(属性)的值（字符类型）
* 设备自身操作状态改变后，需要通过该接口上报给平台状态变化
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*      @did:主/从设备ID
*      @appid:该状态在APP定义的ID值（用户自己定义）
*      @value:状态值
* 返回值:0:成功，< 0:失败
*/
int PlatformPropertySetTextValue(PlatformCtx_t *ctx, const char *did, unsigned short appid, const char *value);

/*
* 设置设备状态(属性)的值（数值类型）
* 设备自身操作状态改变后，需要通过该接口上报给平台状态变化
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*      @did:主/从设备ID
*      @appid:该状态在APP定义的ID值（用户自己定义）
*      @value:状态值
* 返回值:0:成功，< 0:失败
*/
int PlatformPropertySetNumValue(PlatformCtx_t *ctx, const char *did, unsigned short appid, unsigned int value);

/*
* 注册设备状态(属性)
* 操作设备状态(属性)之前需要先进性注册
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*      @did:主/从设备ID
*      @pInfo:设备状态(属性)信息
* 返回值:0:成功，< 0:失败
*/
int PlatformPropertyRegister(PlatformCtx_t *ctx, const char *did, PPropertyInfo_t *pInfo);

/*
* 开始固件升级
* 平台有新版固件时会以事件的形式通知设备，设备选择某一条固件进行升级
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*      @did:主/从设备ID
*      @upgradeInfo:固件信息
*/
void PlatformStartOTA(PlatformCtx_t *ctx, const char *did, PUpgradeInfo_t *upgradeInfo);

/*
* 升级结果上报
* 设备升级完成后上报升级结果
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*      @success:升级结果，true：成功，false：失败
*/
void PlatformOTAResult(PlatformCtx_t *ctx, ptBool_t success);

/*
* 设置设备模块版本号
* 用于平台对比是否有新版本的固件，是否需要推送升级消息
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*      @did:主/从设备ID
*      @name:模块名称
*      @version：版本号
*/
void PlatformSetModuleVersion(PlatformCtx_t *ctx, const char *did, const char *name, const char *version);

/*
* 设置设备信息
* 用于设置设备的主要信息，用于平台登录和鉴权
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*      @did:设备ID,设备在电信智能家居平台的唯一标识
*      @pin:设备PIN码
*      @model:设备型号，内含厂家标识、品牌标识、型号标识等
*      @version:协议版本，版本号格式为 XXX.YYY.ZZZ.MMM
*      @factoryName:厂家名称
*/
void PlatformSetDeviceInfo(PlatformCtx_t *ctx, const char *did, const char *pin, const char *model, const char *version, const char *factoryName);

/*
* 开始执行平台处理业务
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*/
void PlatformStart(PlatformCtx_t *ctx);

/*-----------------------------从设备操作-------------------------------------*/

/*
* 从设备心跳
* 当接收到从设备产生的心跳时，通过该接口更新心跳时间
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*      @did:从设备ID
* 返回值:0:成功，< 0:失败
*/
int PlatformSubDeviceHeartbeat(PlatformCtx_t *ctx, const char *did);

/*
* 从设备接收信号强度（可选）
* 更新从设备的信号强度（更新时间间隔设备可自己定义）
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*      @did:从设备ID
*      @rssi:信号强度
* 返回值:0:成功，< 0:失败
*/
int PlatformSubDeviceRSSIValue(PlatformCtx_t *ctx, const char *did, int rssi);

/*
* 从设备电池电量（可选）
* 更新从设备的电量剩余值（更新时间间隔设备可自己定义）
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*      @did:从设备ID
*      @remain:剩余电量
* 返回值:0:成功，< 0:失败
*/
int PlatformSubDeviceBatteryRemain(PlatformCtx_t *ctx, const char *did, int remain);

/*
* 从设备解绑上报
* 当解绑某个从设备后，需要通过该接口上报给平台
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*      @did:从设备ID
* 返回值:0:成功，< 0:失败
*/
int PlatformSubDeviceUnbind(PlatformCtx_t *ctx, const char *did);

/*
* 从设备上、下线
* 通过该接口上报平台从设备上、下线通知
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*      @did:从设备ID
*      @online:上线标志，true：上线，false：下线
* 返回值:0:成功，< 0:失败
*/
int PlatformSubDeviceOnOffline(PlatformCtx_t *ctx, const char *did, ptBool_t online);

/*
* 从设备信息注册
* 从设备操作之前需要先进性信息注册
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*      @did:从设备ID
*      @pin:从设备PIN码
*      @model:设备型号，内含厂家标识、品牌标识、型号标识等
*      @version:协议版本，版本号格式为 XXX.YYY.ZZZ.MMM
*      @factoryName:厂家名称
* 返回值:0:成功，< 0:失败
*/
int PlatformSubDeviceRegister(PlatformCtx_t *ctx, const char *did, const char *pin, const char *model, const char *version, const char *factoryName);

/*
* 是否连接到电信智能家居平台
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
* 返回值:true：已连接，false：未连接
*/
ptBool_t PlatformServerConnected(PlatformCtx_t *ctx);

/*
* 平台事件回调函数注册
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*      @handle:回调函数
*/
void PlatformEventRegister(PlatformCtx_t *ctx, PlatformEventHandle_t handle);

/*
* 创建平台操作主结构体
* 返回值:!=NULL：主结构体，NULL：创建失败
*/
PlatformCtx_t *PlatformCtxCreate(void);

/*
* 平台初始化函数
*/
void PlatformInitialize(void);

/*
* 平台主循环函数
* 参数：@ctx:主结构体(PlatformCtxCreate返回)
*/
void PlatformPoll(PlatformCtx_t *ctx);

#endif // !PLATFORM_H
