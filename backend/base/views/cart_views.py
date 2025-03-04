from rest_framework.decorators import api_view, permission_classes
from rest_framework.permissions import IsAuthenticated, IsAdminUser
from rest_framework.response import Response

from base.models import Cart

@api_view(['GET'])
@permission_classes([IsAuthenticated])
def getCart(request):
    response = []
    user = request.user
    if not Cart.objects.filter(user=user).exists():
        cart = Cart.objects.create(user=user)
        return Response(response)
    cart = Cart.objects.get(user=user)
    cartList = cart.cartItems[:-1].split(",")
    if cartList != ['']:
        for item in cartList:
            itemId, qty, size = item.split(":")
            product_dict = {
                '_id': itemId,
                'quantity': qty,
                'size': size
            }
            response.append(product_dict)
    return Response(response)

@api_view(['PUT'])
@permission_classes([IsAuthenticated])
def updateCart(request):
    user = request.user
    data = request.data
    cart = Cart.objects.get(user=user)
    cartList = cart.cartItems[:-1].split(",")
    if cartList != ['']:
        for i in range(len(cartList) - 1, -1, -1):
            itemId, _, size = cartList[i].split(":")
            if itemId == str(data["idx"]) and data["qty"] == 0 and size==data["size"]:
                del cartList[i]
            elif itemId == str(data["idx"]) and size == data["size"]:
                cartList[i] = f"{itemId}:{data['qty']}:{size}"
            else:
                pass
    if cartList == []:
        cart.cartItems = ""
    else:
        cart.cartItems = (",".join(cartList) + ",")
    cart.save()
    return Response("Updated", status=201)

@api_view(['POST'])
@permission_classes([IsAuthenticated])
def addToCart(request):
    user = request.user
    data = request.data
    change = False
    if not Cart.objects.filter(user=user).exists():
        cart = Cart.objects.create(user=user, cartItems=f"{data['idx']}:{data['qty']}:{data['size']},")
        return Response("Added", status=200)
    cart = Cart.objects.get(user=user)
    cartList = cart.cartItems[:-1].split(",")
    if cartList != ['']:
        for i in range(len(cartList)):
            itemId, _, size = cartList[i].split(":")
            if itemId == str(data["idx"]) and size == data["size"]:
                cartList[i] = f"{itemId}:{data['qty']}:{data['size']}"
                cart.cartItems = (",".join(cartList) + ",")
                change=True
    if not change:
        if cartList == ['']:
            cart.cartItems = f"{data['idx']}:{data['qty']}:{data['size']},"
        else:
            cartList.append(f"{data['idx']}:{data['qty']}:{data['size']}")
            cart.cartItems = (",".join(cartList) + ",")
    cart.save()
    return Response("Added", status=200)