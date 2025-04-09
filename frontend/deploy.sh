# Script para subir frontend para S3
#!/bin/bash

BUCKET_NAME="frontend-bucket"

aws s3 sync ./frontend/public s3://$BUCKET_NAME --delete
echo "Site publicado em: http://$BUCKET_NAME.s3-website-us-east-1.amazonaws.com"
