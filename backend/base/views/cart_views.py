from rest_framework.decorators import api_view, permission_classes
from rest_framework.permissions import IsAuthenticated, IsAdminUser
from rest_framework.response import Response

from base.models import Cart, Scanner, Product
from base.serializer import ScannerSerializer, CartSerializer
import random

@api_view(['POST'])
def getCart(request):
    response = 0
    id = request.data['id']
    cart = Cart.objects.get(_id=id)
    if cart.cartItems == "":
        return 0
    cartList = cart.cartItems[:-1].split(",")
    if cartList != ['']:
        for item in cartList:
            itemId, qty = item.split(":")
            product = Product.objects.get(_id=itemId)
            response += (product.price * int(qty))
    return Response(response)

@api_view(['GET'])
def getCoords(request):
    coords = Scanner.objects.all()
    serializer = ScannerSerializer(coords, many=True)
    return Response(serializer.data)

@api_view(['POST'])
def updateCoords(request):
    try:
        data = request.data
        id, lat, lng, radius = data['id'], data['lat'], data['lng'], data['radius']
        scanner = Scanner.objects.get(_id=id)
        if str(radius) == "0":
            lat, lng, radius = scanner.location.split(",")
            scanner.location = f"{float(lat)+0.0000001},{float(lng)-0.0000001},{random.choice([15,20,30,40,100,25])}"
        else:
            scanner.location = f"{lat},{lng},{radius}"
        scanner.save()
        return Response("Successful", status=200)
    except Exception as e:
        return Response(str(e), status=400)
    
@api_view(['POST'])
def getLatestTrade(request):
    id = request.data['id']
    cart = Cart.objects.get(_id=id)
    serializer = CartSerializer(cart, many=False)
    return Response(serializer.data)