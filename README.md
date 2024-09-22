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
17) Plist file from single atlas file. Source png file will be copied to destination folder (need fix some cases)

To do:
1) Multiselect
2) Sizing at window (packed picture size more than free space on form)
4) Preview result image

<br/><img src="screen.png" /><br/>

Using in cocos2d-x (single sprite):
```
auto cache = SpriteFrameCache::getInstance();
cache->addSpriteFramesWithFile("anim/grass.plist");
auto frame = cache->getSpriteFrameByName("gr0");
auto sprite = Sprite::createWithSpriteFrame(frame);
sprite->setPosition(300, 300);
sprite->setScale(4.0);
sprite->getTexture()->setAliasTexParameters();
this->addChild(sprite);
```

Using in cocos2d-x (single sprite):
```
auto spritecache = SpriteFrameCache::getInstance();
Vector<SpriteFrame*> animFrames;
spritecache->addSpriteFramesWithFile("anim/fox.plist");
animFrames.pushBack(spritecache->getSpriteFrameByName("fox_pt0"));
animFrames.pushBack(spritecache->getSpriteFrameByName("fox_pt1"));
animFrames.pushBack(spritecache->getSpriteFrameByName("fox_pt2"));
animFrames.pushBack(spritecache->getSpriteFrameByName("fox_pt3"));
cocos2d::Sprite* animSprite_fox = Sprite::createWithSpriteFrame(animFrames.front());
animSprite_fox->setPosition(200, 400);
animSprite_fox->setScale(4.0);
animSprite_fox->setName("anim_fox");
animSprite_fox->setVisible(true);
cocos2d::Animation* animation_fox = Animation::createWithSpriteFrames(animFrames, 0.1);
cocos2d::Animate* animate_fox = Animate::create(animation_fox);
animSprite_fox->runAction(RepeatForever::create(animate_fox));
this->addChild(animSprite_fox, 0);
animFrames.clear();
```

Deploy

<pre>
cd C:\Qt\5.15.2\mingw81_64\bin 
windeployqt E:\QtProjects\build-test1-Desktop_Qt_5_15_2_MinGW_64_bit-Release\release
add libwinpthread-1.dll, libstdc++-6.dll, libgcc_s_seh-1.dll
</pre>
