from django.urls import path
from base.views import cart_views as views
# from rest_framework_simplejwt.views import TokenObtainPairView

urlpatterns = [
    path('', views.getCart, name="cart"),
    path('update/', views.updateCart, name="cart_update"),
    path('add/', views.addToCart, name="cart_add"),
]