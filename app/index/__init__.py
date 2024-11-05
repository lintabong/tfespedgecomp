from flask import Blueprint

Index = Blueprint('index', __name__)

from app.index import index