from django.urls import path
from base.views import product_views as views
# from rest_framework_simplejwt.views import TokenObtainPairView

urlpatterns = [
    path('', views.getProducts, name="products"),
    path('updateProduct/', views.updateProduct, name="product_update"),
    path('updateStock/', views.updateStock, name="stock_update"),
    path('upload/', views.updateImage, name="update_image"),
    path('<str:pk>/', views.getProduct, name="product"),
]