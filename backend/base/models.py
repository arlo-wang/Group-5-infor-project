from django.db import models
# Create your models here.

class Product(models.Model):
    name = models.CharField(max_length=200, null=True, blank=True)
    barcode = models.CharField(max_length=200, null=True, blank=True)
    stock = models.IntegerField(null=True, blank=True, default=0)
    price = models.DecimalField(max_digits=7, decimal_places=2, null=True, blank=True)
    _id = models.AutoField(primary_key=True, editable=False)

    def __str__(self):
        return str(self.name)

class Scanner(models.Model):
    location = models.CharField(max_length=200, null=True, blank=True)
    _id = models.AutoField(primary_key=True, editable=False)

    def __str__(self):
        return str(self._id)
    
class Cart(models.Model):
    scanner = models.OneToOneField(Scanner, on_delete=models.CASCADE, null=True)
    cartItems = models.TextField(null=True, blank=True, default="") # product_id:qty,product_id2:qty2
    _id = models.AutoField(primary_key=True, editable=False)

    def __str__(self):
        return (str(self._id))