PNG files packer.

Passed:
1) Remove fill color from image - OK
2) Crop image (check transparent pixels) - OK
3) Create packed image - OK
4) Export to plist format - OK

Need test:
1) Save changes (remove fill color and crop) at session
2) Apply changes to all pictures
3) Sizing at window (result picture more than free space on form)

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
