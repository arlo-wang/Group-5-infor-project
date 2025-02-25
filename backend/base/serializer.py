from rest_framework import serializers
from django.contrib.auth.models import User
from rest_framework_simplejwt.tokens import RefreshToken
from .models import Product, Order, OrderItem, ShippingAddress, ProductStock

from datetime import datetime

class ProductStockSerializer(serializers.ModelSerializer):
    class Meta:
        model = ProductStock
        fields = '__all__'

class ProductSerializer(serializers.ModelSerializer):

    stocks = ProductStockSerializer(many=True, read_only=True)
    class Meta:
        model = Product
        fields = '__all__'

class ShippingAddressSerializer(serializers.ModelSerializer):
    class Meta:
        model = ShippingAddress
        fields = '__all__'

class OrderItemSerializer(serializers.ModelSerializer):
    class Meta:
        model = OrderItem
        fields = '__all__'

class OrderSerializer(serializers.ModelSerializer):
    orders = serializers.SerializerMethodField(read_only=True)
    shippingAddress = serializers.SerializerMethodField(read_only=True)
    user = serializers.SerializerMethodField(read_only=True)
    deliveredAt = serializers.SerializerMethodField(read_only=True)
    paidAt = serializers.SerializerMethodField(read_only=True)

    class Meta:
        model = Order
        fields = ['user', 'paymentMethod', 'totalPrice', 'isPaid', 'paidAt', 'isDelivered', 'deliveredAt', 'createdAt', 'shippingAddress', 'orders', '_id']

    def get_paidAt(self, obj):
        if obj.isPaid:
            paid = obj.paidAt
            paidStr = paid.strftime("%Y-%m-%d")
        else:
            paidStr = ""
        return paidStr
    
    def get_deliveredAt(self, obj):
        if obj.isDelivered:
            delivered = obj.deliveredAt
            deliveredStr = delivered.strftime("%Y-%m-%d")
        else:
            deliveredStr = ""
        return deliveredStr

    def get_orders(self, obj):
        items = obj.orderitem_set.all()
        serializers = OrderItemSerializer(items, many=True)
        return serializers.data
    
    def get_shippingAddress(self, obj):
        try:
            address = ShippingAddressSerializer(obj.shippingaddress, many=False).data
        except:
            address = False
        return address
    
    def get_user(self, obj):
        user = obj.user
        serializers = UserSerializer(user, many=False)
        return serializers.data

class UserSerializer(serializers.ModelSerializer):
    name = serializers.SerializerMethodField(read_only=True)
    is_admin = serializers.SerializerMethodField(read_only=True)

    class Meta:
        model = User
        fields = ['id', 'username', 'email', 'name', 'is_admin']
    
    def get_is_admin(self, obj):
        return obj.is_staff

    def get_name(self, obj):
        name = obj.first_name
        if name == '':
            name = obj.email
        return name
    
class UserSerializerWithToken(UserSerializer):
    token = serializers.SerializerMethodField(read_only=True)

    class Meta:
        model = User
        fields = ['id', 'username', 'email', 'name', 'is_admin', 'token']
    
    def get_token(self, obj):
        token = RefreshToken.for_user(obj)
        return str(token.access_token)