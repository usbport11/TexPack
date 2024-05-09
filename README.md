PNG files packer (create texture atlas).

Passed:
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

To do:
1) Apply changes to all pictures
2) Sizing at window (packed picture size more than free space on form)
3) Remove/add pictures in list view (half)
4) Preview result image
5) Crop by pixels number

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
