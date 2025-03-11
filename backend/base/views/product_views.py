from rest_framework.decorators import api_view, permission_classes
from rest_framework.response import Response

from base.models import Product, Cart
import urllib, base64

from base.serializer import ProductSerializer

import urllib.parse
import base64
from PIL import Image
from pyzbar.pyzbar import decode
import os, uuid
import random

@api_view(['POST'])
def getProducts(request):
    data = request.POST.get('image')
    id = request.POST.get('id')
    url_decoded_data = urllib.parse.unquote(data)
    image_data = base64.b64decode(url_decoded_data)
    if os.path.exists("image.jpg"):
        os.remove("image.jpg")
    with open("image.jpg", 'wb') as image_file:
        image_file.write(image_data)
    image = Image.open("image.jpg")
    decoded_objects = decode(image)
    lst = []
    for obj in decoded_objects:
        product = Product.objects.filter(barcode=obj.data.decode('utf-8')).first()
        if product:
            lst.append(product)
    if len(lst):
        product = lst[0]
    else:
        items = list(Product.objects.all())
        product = random.choice(items)
    change = False
    cart = Cart.objects.get(_id=id)
    cartList = cart.cartItems[:-1].split(',')
    if cartList != ['']:
        for i in range(len(cartList)):
            itemId, qty = cartList[i].split(":")
            if itemId == str(product._id):
                cartList[i] = f"{itemId}:{int(qty)+1}" # add item
                cart.cartItems = (",".join(cartList) + ",")
                change=True
    if not change:
        if cartList == ['']:
            cart.cartItems = f"{product._id}:1,"
        else:
            cartList.append(f"{product._id}:1")
            cart.cartItems = (",".join(cartList) + ",")
    cart.latestTrade = str(uuid.uuid4())
    cart.latestTradeAmount = product.price
    cart.save()
    serializer = ProductSerializer(product, many=False)
    return Response(serializer.data)