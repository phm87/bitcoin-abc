/******************************************************************************
 * Copyright © 2014-2019 The SuperNET Developers.                             *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * SuperNET software, including this file may be copied, modified, propagated *
 * or distributed except according to the terms contained in the LICENSE file *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/


#ifndef komodo_rpcblockchain_h
#define komodo_rpcblockchain_h

int32_t komodo_MoMdata(int32_t *notarized_htp,uint256 *MoMp,uint256 *kmdtxidp,int32_t height,uint256 *MoMoMp,int32_t *MoMoMoffsetp,int32_t *MoMoMdepthp,int32_t *kmdstartip,int32_t *kmdendip);
uint256 komodo_calcMoM(int32_t height,int32_t MoMdepth);
extern char ASSETCHAINS_SYMBOL[65];
uint32_t DPOWCONFS = 1;
extern int32_t NOTARIZED_HEIGHT;

int32_t komodo_dpowconfs(int32_t txheight,int32_t numconfs)
{
    // DPoW confs are on by default
    int32_t dpowconfs = 1;
    DPOWCONFS = gArgs.GetArg("-dpowconfs",dpowconfs);
    if ( DPOWCONFS != 0 && txheight > 0 && numconfs > 0 )
    {
        if ( NOTARIZED_HEIGHT > 0 )
        {
            if ( txheight < NOTARIZED_HEIGHT )
                return(numconfs);
            else return(1);
        }
    }
    return(numconfs);
}

int32_t komodo_MoM(int32_t *notarized_heightp,uint256 *MoMp,uint256 *kmdtxidp,int32_t nHeight,uint256 *MoMoMp,int32_t *MoMoMoffsetp,int32_t *MoMoMdepthp,int32_t *kmdstartip,int32_t *kmdendip)
{
    int32_t depth,notarized_ht; uint256 MoM,kmdtxid;
    depth = komodo_MoMdata(&notarized_ht,&MoM,&kmdtxid,nHeight,MoMoMp,MoMoMoffsetp,MoMoMdepthp,kmdstartip,kmdendip);
    memset(MoMp,0,sizeof(*MoMp));
    memset(kmdtxidp,0,sizeof(*kmdtxidp));
    *notarized_heightp = 0;
    if ( depth > 0 && notarized_ht > 0 && nHeight > notarized_ht-depth && nHeight <= notarized_ht )
    {
        *MoMp = MoM;
        *notarized_heightp = notarized_ht;
        *kmdtxidp = kmdtxid;
    }
    return(depth);
}

static UniValue calc_MoM(const Config &config, const JSONRPCRequest& request)
{
    int32_t height,MoMdepth; uint256 MoM; UniValue ret(UniValue::VOBJ); UniValue a(UniValue::VARR);
    if ( request.params.size() != 2 )
        throw std::runtime_error("calc_MoM height MoMdepth\n");
    LOCK(cs_main);
    height = atoi(request.params[0].get_str().c_str());
    MoMdepth = atoi(request.params[1].get_str().c_str());
    if ( height <= 0 || MoMdepth <= 0 || MoMdepth >= height )
        throw std::runtime_error("calc_MoM illegal height or MoMdepth\n");
    //fprintf(stderr,"height_MoM height.%d\n",height);
    MoM = komodo_calcMoM(height,MoMdepth);
    ret.push_back(Pair("coin",(char *)(ASSETCHAINS_SYMBOL[0] == 0 ? "KMD" : ASSETCHAINS_SYMBOL)));
    ret.push_back(Pair("height",height));
    ret.push_back(Pair("MoMdepth",MoMdepth));
    ret.push_back(Pair("MoM",MoM.GetHex()));
    return ret;
}

static UniValue height_MoM(const Config &config, const JSONRPCRequest& request)
{
    int32_t height,depth,notarized_height,MoMoMdepth,MoMoMoffset,kmdstarti,kmdendi; uint256 MoM,MoMoM,kmdtxid; uint32_t timestamp = 0; UniValue ret(UniValue::VOBJ); UniValue a(UniValue::VARR);
    if ( request.params.size() != 1 )
        throw std::runtime_error("height_MoM height\n");
    LOCK(cs_main);
    height = atoi(request.params[0].get_str().c_str());
    if ( height <= 0 )
    {
        if ( chainActive.Tip() == 0 )
        {
            ret.push_back(Pair("error",(char *)"no active chain yet"));
            return(ret);
        }
        height = chainActive.Tip()->nHeight;
    }
    //fprintf(stderr,"height_MoM height.%d\n",height);
    depth = komodo_MoM(&notarized_height,&MoM,&kmdtxid,height,&MoMoM,&MoMoMoffset,&MoMoMdepth,&kmdstarti,&kmdendi);
    ret.push_back(Pair("coin",(char *)(ASSETCHAINS_SYMBOL[0] == 0 ? "KMD" : ASSETCHAINS_SYMBOL)));
    ret.push_back(Pair("height",height));
    ret.push_back(Pair("timestamp",(uint64_t)timestamp));
    if ( depth > 0 )
    {
        ret.push_back(Pair("depth",depth));
        ret.push_back(Pair("notarized_height",notarized_height));
        ret.push_back(Pair("MoM",MoM.GetHex()));
        ret.push_back(Pair("kmdtxid",kmdtxid.GetHex()));
        if ( ASSETCHAINS_SYMBOL[0] != 0 )
        {
            ret.push_back(Pair("MoMoM",MoMoM.GetHex()));
            ret.push_back(Pair("MoMoMoffset",MoMoMoffset));
            ret.push_back(Pair("MoMoMdepth",MoMoMdepth));
            ret.push_back(Pair("kmdstarti",kmdstarti));
            ret.push_back(Pair("kmdendi",kmdendi));
        }
    } else ret.push_back(Pair("error",(char *)"no MoM for height"));
    
    return ret;
}

#endif /* komodo_rpcblockchain_h */
