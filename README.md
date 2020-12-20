PNG files packer (create texture atlas).

Passed:
1) Remove fill color from image - OK
2) Crop image (check transparent pixels) - OK
3) Create packed image - OK
4) Export to plist format - OK

Need test:
1) Save changes (remove fill color and crop) at session
2) Apply changes to all pictures
3) Sizing at window (packed picture size more than free space on form)
4) Naming frames inside plist file by png files names
5) Remove/add pictures in list view

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
