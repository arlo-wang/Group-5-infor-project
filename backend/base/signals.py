from django.db.models.signals import pre_save, post_save
from django.contrib.auth.models import User
from .models import ShoppingCart, ProductStock, Product

"""
def updateUser(sender, instance, **kwargs):
    user = instance
    if user.email != '':
        user.username = user.email
"""

def createShoppingCart(sender, instance, **kwargs):
    user = instance
    try:
        _ = ShoppingCart.objects.get_or_create(user=user)
    except:
        pass

def ensure_product_stock(sender, instance, **kwargs):
    for size in range(instance.min_size, instance.max_size + 1, 5):
        if not instance.stocks.filter(size=size).exists():
            ProductStock.objects.create(product=instance, size=size, countInStock=0)

#pre_save.connect(updateUser, sender=User)
post_save.connect(createShoppingCart, sender=User)
post_save.connect(ensure_product_stock, sender=Product)