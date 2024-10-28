import { TerraformStack, TerraformVariable } from "cdktf";
import { S3Bucket } from "@cdktf/provider-aws/lib/s3-bucket";
import { AcmCertificateValidation } from "@cdktf/provider-aws/lib/acm-certificate-validation";
import { CloudfrontDistribution } from "@cdktf/provider-aws/lib/cloudfront-distribution";
import { CloudfrontOriginAccessControl } from "@cdktf/provider-aws/lib/cloudfront-origin-access-control";
import { S3BucketPolicy } from "@cdktf/provider-aws/lib/s3-bucket-policy";

export function configureCloudFront(
  stack: TerraformStack,
  staticWebsiteStorageBucket: S3Bucket,
  certificateValidation: AcmCertificateValidation,
  domainName: TerraformVariable,
  resourceTags?: any,
): CloudfrontDistribution {
  // create the cloudfront origin access control
  const cloudfrontOac = new CloudfrontOriginAccessControl(
    stack,
    "website-cloudfront-origin-access-control",
    {
      description: "Website CloudFront Origin Access Control",
      name: "website-cloudfront-origin-access-control",
      originAccessControlOriginType: "s3",
      signingBehavior: "always",
      signingProtocol: "sigv4",
    },
  );

  // create the cloudfront distribution
  const cloudfrontDistribution = new CloudfrontDistribution(
    stack,
    "website-cloudfront-distribution",
    {
      dependsOn: [
        staticWebsiteStorageBucket,
        certificateValidation,
        cloudfrontOac,
      ],
      enabled: true,
      aliases: [domainName.value],
      isIpv6Enabled: true,
      defaultRootObject: "index.html",
      customErrorResponse: [
        {
          errorCode: 403,
          responseCode: 200,
          responsePagePath: "/index.html",
        },
        {
          errorCode: 404,
          responseCode: 200,
          responsePagePath: "/index.html",
        },
      ],
      defaultCacheBehavior: {
        allowedMethods: ["GET", "HEAD", "OPTIONS"],
        cachedMethods: ["GET", "HEAD", "OPTIONS"],
        targetOriginId: `S3-${staticWebsiteStorageBucket.id}`,
        viewerProtocolPolicy: "redirect-to-https",
        forwardedValues: {
          queryString: false,
          cookies: {
            forward: "none",
          },
        },
        minTtl: 0,
        defaultTtl: 3600,
        maxTtl: 86400,
        compress: true,
        fieldLevelEncryptionId: "",
      },
      orderedCacheBehavior: [],
      origin: [
        {
          originId: `S3-${staticWebsiteStorageBucket.id}`,
          domainName: staticWebsiteStorageBucket.bucketRegionalDomainName,
          originAccessControlId: cloudfrontOac.id,
        },
      ],
      restrictions: {
        geoRestriction: {
          restrictionType: "whitelist",
          locations: ["US", "VI", "PR"],
        },
      },
      viewerCertificate: {
        acmCertificateArn: certificateValidation.certificateArn,
        sslSupportMethod: "sni-only",
        minimumProtocolVersion: "TLSv1.1_2016",
      },
      priceClass: "PriceClass_100",
      tags: {
        ...resourceTags,
      },
    },
  );
  // Add the cloudfront distribution to the bucket policy
  new S3BucketPolicy(stack, "website-static-website-storage-bucket-policy", {
    dependsOn: [staticWebsiteStorageBucket],
    bucket: staticWebsiteStorageBucket.bucket,
    policy: JSON.stringify({
      Version: "2012-10-17",
      Statement: [
        {
          Sid: "AllowCloudFrontServicePrincipalReadOnly",
          Effect: "Allow",
          Principal: {
            Service: "cloudfront.amazonaws.com",
          },
          Action: "s3:GetObject",
          Resource: `arn:aws:s3:::${staticWebsiteStorageBucket.bucket}/*`,
          Condition: {
            StringEquals: {
              "AWS:SourceArn": cloudfrontDistribution.arn,
            },
          },
        },
      ],
    }),
  });

  return cloudfrontDistribution;
}
