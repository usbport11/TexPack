PNG files packer (create texture atlas) with plist file.

Ready:
1) Remove background color from image (top left pixel as base color)
2) Crop image by transparent background pixels (usually using after previous step)
3) Pack images in one image
4) Export to plist format
5) Fix artifacts while image create, remove back and crop
6) Add Reset, Select Directory, Path label, Alpha value
7) Remove image in list by DEL key
8) Restore image in list by R key
9) Naming frames inside plist file by png files names or prefix
10) Preview selected source image (result image once on finish)
11) Reset all actions
12) Crop by pixels number
13) Open files now add files without remove existed files in list
14) Settings file (load at start, save on exit)
15) Option to pack images simple way (one by one) with column number
16) Fix pack order while pack by compact method
17) Plist file from single atlas file (not ready)

To do:
1) Multiselect
2) Sizing at window (packed picture size more than free space on form)
4) Preview result image

<br/><img src="screen.png" /><br/>

Using in cocos2d-x:
<pre>
SpriteFrameCache* cache = SpriteFrameCache::getInstance();
cache->addSpriteFramesWithFile("resources/out.plist");

auto visibleSize = Director::getInstance()->getVisibleSize();
Vec2 origin = Director::getInstance()->getVisibleOrigin();

auto frame = a->getSpriteFrameByName("pt0");
auto sprite = Sprite::createWithSpriteFrame(frame);
sprite->setPosition(0, 0);
sprite->setAnchorPoint(Vec2(0,0));
sprite->retain();
this->addChild(sprite);
</pre>

<br/>

Deploy

<pre>
cd C:\Qt\5.15.2\mingw81_64\bin 
windeployqt E:\QtProjects\build-test1-Desktop_Qt_5_15_2_MinGW_64_bit-Release\release
add libwinpthread-1.dll, libstdc++-6.dll, libgcc_s_seh-1.dll
</pre>
