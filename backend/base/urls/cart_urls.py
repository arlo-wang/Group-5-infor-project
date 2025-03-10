from django.urls import path
from base.views import cart_views as views
# from rest_framework_simplejwt.views import TokenObtainPairView

urlpatterns = [
    path('', views.getCart, name="cart"),
    path('location/', views.getCoords, name='coords'),
    path('update/', views.updateCoords, name='update-coords'),
    path('latest/', views.getLatestTrade, name='latest'),
]