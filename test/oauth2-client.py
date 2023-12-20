#!/usr/bin/python3
# -*- coding: utf-8 -*-

import requests

def get_access_token(url, client_id, client_secret):
    response = requests.post(
        url,
        data={"grant_type": "client_credentials"},
        auth=(client_id, client_secret),
    )
    return response.json()["access_token"]


get_access_token("http://127.0.0.1:8989/oauth2/authorize", "abcde", "12345")

