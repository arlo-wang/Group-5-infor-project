from rest_framework.decorators import api_view, permission_classes
from rest_framework.response import Response

from base.models import Product
import urllib, base64

from base.serializer import ProductSerializer

import urllib.parse
import base64
from PIL import Image
from io import BytesIO
from pyzbar.pyzbar import decode

@api_view(['POST'])
def getProducts(request):
    data = request.POST.get('image')
    url_decoded_data = urllib.parse.unquote(data)
    image_data = base64.b64decode(url_decoded_data)
    image = Image.open(BytesIO(image_data))
    decoded_objects = decode(image)
    lst = []
    for obj in decoded_objects:
        product = Product.objects.filter(barcode=obj.data.decode('utf-8')).first()
        if product:
            lst.append(product)
    product = lst[0]
    serializer = ProductSerializer(product, many=False)
    return Response(serializer.data)