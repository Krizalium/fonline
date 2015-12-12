#ifndef __ITEM_HEX__
#define __ITEM_HEX__

#include "Defines.h"
#include "Item.h"
#include "SpriteManager.h"

#define EFFECT_0_TIME_PROC    ( 10 )
#define EFFECT_0_SPEED_MUL    ( 10.0f )

struct AnyFrames;

class ItemHex: public Item
{
private:
    ItemHex( uint id, ProtoItem* proto );
    void AfterConstruction();

public:
    ItemHex( uint id, ProtoItem* proto, Properties& props );
    ItemHex( uint id, ProtoItem* proto, UCharVecVec* props_data );
    ItemHex( uint id, ProtoItem* proto, UCharVecVec* props_data, int hx, int hy, int* hex_scr_x, int* hex_scr_y );

public:
    uint       SprId;
    int        HexX, HexY;
    short      ScrX, ScrY;
    int*       HexScrX, * HexScrY;
    uchar      Alpha;
    AnyFrames* Anim;
    bool       SprDrawValid;
    Sprite*    SprDraw, * SprTemp;
    Effect*    DrawEffect;

private:
    int   curSpr, begSpr, endSpr;
    uint  animBegSpr, animEndSpr;
    uint  animTick;
    uchar maxAlpha;
    bool  isAnimated;
    uint  animNextTick;

public:
    bool IsCanUseSkill()      { return IsScenOrGrid() || IsItem(); }
    bool IsScenOrGrid()       { return IsScen() || IsGrid(); }
    bool IsAnimated()         { return isAnimated; }
    bool IsCanLook()          { return !( IsGrid() && GetGrid_Type() == GRID_EXITGRID ); }
    bool IsUsable()           { return !IsWall() && ( GetIsCanUse() || GetIsCanUseOnSmth() || GetIsCanPickUp() ); }
    bool IsTalkable()         { return !IsWall() && GetIsCanTalk(); }
    bool IsDrawContour()      { return /*IsFocused && */ IsItem() && !GetIsNoHighlight() && !GetIsBadItem(); }
    bool IsTransparent()      { return maxAlpha < 0xFF; }
    bool IsFullyTransparent() { return maxAlpha == 0; }
    void RefreshAnim();
    void RestoreAlpha() { Alpha = maxAlpha; }
    void RefreshAlpha() { maxAlpha = ( IsColorize() ? GetAlpha() : 0xFF ); }
    void SetSprite( Sprite* spr );
    int  GetEggType();

    // Finish
private:
    bool finishing;
    uint finishingTime;

public:
    void Finish();
    bool IsFinishing() { return finishing; }
    bool IsFinish()    { return ( finishing && Timer::GameTick() > finishingTime ); }
    void StopFinishing();

    // Process
public:
    void Process();

    // Effect
private:
    bool  isEffect;
    float effSx, effSy;
    int   effStartX, effStartY;
    float effCurX, effCurY;
    uint  effDist;
    uint  effLastTick;
    int   effDir;

public:
    float EffOffsX, EffOffsY;

    bool       IsDynamicEffect() { return isEffect && ( effSx || effSy ); }
    void       SetEffect( float sx, float sy, uint dist, int dir );
    UShortPair GetEffectStep();

    // Fade
private:
    bool fading;
    uint fadingTick;
    bool fadeUp;

    void SetFade( bool fade_up );

    // Animation
public:
    void StartAnimate();
    void StopAnimate();
    void SetAnimFromEnd();
    void SetAnimFromStart();
    void SetAnim( uint beg, uint end );
    void SetSprStart();
    void SetSprEnd();
    void SetSpr( uint num_spr );
    void SetAnimOffs();
    void SetStayAnim();
    void SetShowAnim();
    void SetHideAnim();

public: // Move some specific types to end
    UShortPairVec EffSteps;
};

typedef vector< ItemHex* > ItemHexVec;

#endif // __ITEM_HEX__
