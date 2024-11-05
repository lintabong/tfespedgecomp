from flask import Blueprint

Admin = Blueprint('admin', __name__)

from app.admin import dashboard